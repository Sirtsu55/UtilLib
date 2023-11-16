#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <functional>
#include <cstring>

#include "Function.h"
#include "JobSystem.h"

class Task
{
public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    /// @brief Construct a new Task object
    /// @param name Name of the task
    /// @param function Function to execute when the task is called
    /// @param data Data to pass to the function
    Task(const std::string& name, const Function<void(void*)>& function, void* data = nullptr)
        : mName(name), mHash(hasher(name)), mFunction(function), mData(data)
    {
    }

    Task() = default;

    ~Task() = default;

    /// --------------------------------------------------------
    /// Getters & Setters
    /// --------------------------------------------------------

    /// @brief Get the name of the task
    /// @return Name of the task
    const std::string& GetName() const { return mName; }

    /// @brief Get the hash of the task
    /// @return Hash of the task
    uint64_t GetHash() const { return mHash; }

    /// @brief Get the data that is passed to the function
    /// @return Pointer to the data
    void* GetData() const { return mData; }

    /// @brief Get the dependencies of the task
    /// @return Dependencies of the task
    const std::unordered_set<uint64_t>& GetDependencies() const { return mDependencies; }

    /// --------------------------------------------------------
    /// Task Management
    /// --------------------------------------------------------

    /// @brief Add a dependency to the task
    /// @param hash Hash of the task to add as a dependency
    void AddDependency(const uint64_t hash) { mDependencies.insert(hash); }

    /// @brief Add a dependency to the task
    /// @param name Name of the task to add as a dependency
    void AddDependency(const std::string& name) { mDependencies.insert(hasher(name)); }

    /// @brief Execute the task
    void Execute(void* data) { mFunction(data); }

private:
    /// @brief Name of the task
    std::string mName;

    /// @brief Hash of the task
    uint64_t mHash;

    /// @brief Function to execute when the task is called
    Function<void(void*)> mFunction;

    /// @brief Data to pass to the function
    void* mData;

    /// @brief Hashed dependencies
    std::unordered_set<uint64_t> mDependencies;

    inline static std::hash<std::string> hasher = std::hash<std::string> {};

    friend class TaskGraphManager;
};

class TaskGraphManager
{
private:
    struct TaskNodeHeader
    {
        // Header
        uint64_t LeafCount = 0;
        uint64_t TaskHash;
        void* Data;
        std::atomic<bool> Executed;
        // Header End
    };

    template<uint32_t N>
    struct TaskNode // This struct isn't used, but here to help visualize the memory layout
    {
        TaskNodeHeader Header;
        // Leaf nodes to execute
        uint64_t Leaves[N];
    };

public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    TaskGraphManager() = default;
    TaskGraphManager(const TaskGraphManager& other) = delete;
    TaskGraphManager(TaskGraphManager&& other) = delete;

    ~TaskGraphManager()
    {
        for (auto& it : mGraph)
        {
            auto& node = it.second;
            free(node);
        }
    }

    /// --------------------------------------------------------
    /// Task Management
    /// --------------------------------------------------------

    /// @brief Add a task to the graph
    /// @param task Task to add
    void AddTask(const Task& task) { mTasks.emplace(task.GetHash(), task); }

    /// @brief Remove a task from the graph, if the task is dependent on other tasks, building the graph will throw an
    /// exception
    /// @param name Hash of the task to remove
    void RemoveTask(const std::string& name) { RemoveTask(Task::hasher(name)); }

    /// @brief Remove a task from the graph, if the task is dependent on other tasks, building the graph will throw an
    /// exception
    /// @param hash Hash of the task to remove
    void RemoveTask(const uint64_t hash)
    {
        // Check if the task exists
        if (mTasks.find(hash) == mTasks.end())
            return;
        // Remove the task
        mTasks.erase(hash);
    }

    /// @brief Build the graph. In the case where there are circular dependencies, the graph will be built, but the
    /// will lead to undefined behaviour when executed. The graph will be built from scratch, so if you add a task
    /// after building the graph, you will have to rebuild the graph.
    void BuildGraph()
    {
        const uint64_t headerSize = sizeof(TaskNodeHeader);
        const uint64_t leafSize = sizeof(uint64_t);
        std::vector<uint64_t> allDependencies = {};

        ClearGraph();

        // Build the graph from scratch
        for (auto& it : mTasks)
        {
            auto hash = it.first;
            auto& task = it.second;

            // Get task dependencies
            auto& dependencies = task.GetDependencies();
            allDependencies.insert(allDependencies.end(), dependencies.begin(), dependencies.end());
            uint64_t leafCount = dependencies.size();

            // Allocate memory for the node
            // TODO: Add option to use a memory pool
            uint64_t nodeSize = headerSize + (leafCount * leafSize);
            char* node = reinterpret_cast<char*>(malloc(nodeSize));
            void* data = task.GetData();

            TaskNodeHeader* header = reinterpret_cast<TaskNodeHeader*>(node);

            header->LeafCount = leafCount;
            header->TaskHash = hash;
            header->Data = data;
            header->Executed = false;

            // Write leaf nodes
            uint32_t leafIndex = 0;
            for (auto& leaf : dependencies)
            {
                memcpy(node + headerSize + (leafIndex * leafSize), &leaf, sizeof(uint64_t));
                leafIndex++;
            }

            // Validate that all dependencies exist
            for (auto& leaf : dependencies)
            {
                if (mTasks.find(leaf) == mTasks.end())
                {
                    throw std::runtime_error("TaskGraphManager: Task " + task.GetName() +
                                             " has a dependency that does not exist!");
                }
            }

            // Add node to the graph
            mGraph.emplace(hash, node);
        }

        // Find all top nodes
        for (auto& it : mGraph)
        {
            auto hash = it.first;
            auto& node = it.second;

            // If the node is not a dependency of any other node, it is a top node
            if (std::find(allDependencies.begin(), allDependencies.end(), hash) == allDependencies.end())
            {
                mTopNodes.push_back(node);
            }
        }
    }

    /// @brief Execute the graph
    void ExecuteGraph()
    {
        for (char* node : mTopNodes) { ExecuteNode(node); }
    }

    /// @brief Execute the graph in parallel. This doesn't pause execution on this thread, but instead uses the job
    /// system to execute the graph in parallel. So the user will have to wait for the job system to finish.
    /// @param jobSystem Job system to use
    void ExecuteGraphParallel(JobSystem& jobSystem)
    {
        for (char* node : mTopNodes)
        {
            jobSystem.AddJob(Function<void(void*)>(this, &TaskGraphManager::ExecuteNode), node);
        }
    }

private:
    /// @brief Execute a node, for recursive calls
    void ExecuteNode(void* node)
    {
        // Get the header
        TaskNodeHeader* header = reinterpret_cast<TaskNodeHeader*>(node);

        // Optimization: If the task has already been executed, don't execute it again
        // and go through the leaf nodes
        if (header->Executed)
            return;
        // Set the task as executedright after checking if it has been executed
        // to avoid threads executing the same task
        header->Executed = true;

        // Get the leaf count
        const uint64_t leafCount = header->LeafCount;

        // Get the leaf nodes
        uint64_t* leaves = reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(node) + sizeof(TaskNodeHeader));

        // Execute the leaf nodes
        for (uint64_t i = 0; i < leafCount; i++)
        {
            auto leaf = leaves[i];
            auto& leafNode = mGraph[leaf];
            ExecuteNode(leafNode);
        }

        // Execute the task after all leaf nodes have been executed
        auto& task = mTasks[header->TaskHash];
        task.Execute(task.GetData());
    }

    void ClearGraph()
    {
        for (auto& it : mGraph)
        {
            auto& node = it.second;
            free(node);
        }

        mGraph.clear();
        mTopNodes.clear();
    }

private:
    /// @brief The tasks
    std::unordered_map<uint64_t, Task> mTasks;

    /// @brief The graph
    std::unordered_map<uint64_t, char*> mGraph;

    /// @brief The Top Nodes that have dependencies
    std::vector<char*> mTopNodes;
};

// Example usage:

// void Starting(void* data)
//{
//     std::cout << "Starting Program!" << std::endl;
// }
//
// void sayHello(void* data)
//{
//     const char* name = reinterpret_cast<const char*>(data);
//     std::cout << "Hello, " << name << "!" << std::endl;
// }
//
// void sayExitting(void* data)
//{
//     std::cout << "Exitting..." << std::endl;
// }
//
// void printGoodbye(void* data)
//{
//     std::cout << "Goodbye World!" << std::endl;
// }
//
// int main()
//{
//     const char* name = "John Doe";
//
//     Task sayHelloTask("Say Hello", Function<void(void*)>(sayHello), const_cast<char*>(name));
//     Task sayExittingTask("Say Exitting", Function<void(void*)>(sayExitting));
//     Task printGoodbyeTask("Print Goodbye", Function<void(void*)>(printGoodbye));
//     Task startingTask("Starting", Function<void(void*)>(Starting));
//
//     sayHelloTask.AddDependency(startingTask.GetHash());
//     sayExittingTask.AddDependency(sayHelloTask.GetHash());
//     sayExittingTask.AddDependency(printGoodbyeTask.GetHash());
//
//     TaskGraphManager taskGraphManager;
//
//     taskGraphManager.AddTask(startingTask);
//     taskGraphManager.AddTask(sayHelloTask);
//     taskGraphManager.AddTask(sayExittingTask);
//     taskGraphManager.AddTask(printGoodbyeTask);
//
//     taskGraphManager.BuildGraph();
//
//     taskGraphManager.ExecuteGraph();
// }
