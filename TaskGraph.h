#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include "Function.h"

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
        // Header End
    };

    template<uint32_t N>
    struct TaskNode // pseudo code for a node, this struct isn't used, but here to help visualize the memory layout
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

    /// --------------------------------------------------------
    /// Task Management
    /// --------------------------------------------------------

    /// @brief Add a task to the graph
    /// @param task Task to add
    void AddTask(const Task& task) { mTasks.emplace(task.GetHash(), task); }

    /// @brief Build the graph
    void BuildGraph()
    {
        uint64_t taskCount = mTasks.size();
        const uint64_t headerSize = sizeof(TaskNodeHeader);
        const uint64_t leafSize = sizeof(uint64_t);
        std::vector<uint64_t> allDependencies;

        for (auto& it : mTasks)
        {
            auto hash = it.first;
            auto& task = it.second;

            // Get task dependencies
            auto& dependencies = task.GetDependencies();
            allDependencies.insert(allDependencies.end(), dependencies.begin(), dependencies.end());
            uint64_t leafCount = dependencies.size();

            // Allocate memory for the node
            uint64_t nodeSize = headerSize + (leafCount * leafSize);
            char* node = reinterpret_cast<char*>(malloc(nodeSize));
            void* data = task.GetData();

            // Write header
            memcpy(node, &leafCount, sizeof(uint64_t));
            memcpy(node + offsetof(TaskNodeHeader, TaskHash), &hash, sizeof(uint64_t));
            memcpy(node + offsetof(TaskNodeHeader, Data), &data, sizeof(void*));

            // Write leaf nodes
            uint32_t leafIndex = 0;
            for (auto& leaf : dependencies)
            {
                memcpy(node + headerSize + (leafIndex * leafSize), &leaf, sizeof(uint64_t));
                leafIndex++;
            }

            // Add node to the graph
            mGraph.emplace(hash, node);
        }

        // Find all top nodes
        for (auto& it : mGraph)
        {
            auto hash = it.first;
            auto& node = it.second;

            if (std::find(allDependencies.begin(), allDependencies.end(), hash) == allDependencies.end())
            {
                mTopNodes.push_back(node);
            }
        }

#ifndef NDEBUG
        // Check for cycles
        for (auto& it : mGraph)
        {
            const auto hash = it.first;
            const auto& node = it.second;

            // Get leaf count
            uint64_t leafCount = 0;
            memcpy(&leafCount, node, sizeof(uint64_t));

            // Get leaf nodes
            uint64_t* leaves = reinterpret_cast<uint64_t*>(node + headerSize);

            // Check for cycles
            for (uint64_t i = 0; i < leafCount; i++)
            {
                auto leaf = leaves[i];
                if (leaf == hash)
                {
                    std::runtime_error("TaskGraphManager: Cycle detected in graph!");
                    return;
                }
            }
        }
#endif // NDEBUG
    }

    /// @brief Execute the graph
    void ExecuteGraph()
    {
        for (char* node : mTopNodes) { ExecuteNode(node); }
    }

private:
    /// @brief Execute a node, for recursive calls
    void ExecuteNode(char* node)
    {
        // Get the header
        TaskNodeHeader* header = reinterpret_cast<TaskNodeHeader*>(node);

        // Get the leaf count
        const uint64_t leafCount = header->LeafCount;

        // Get the leaf nodes
        uint64_t* leaves = reinterpret_cast<uint64_t*>(node + sizeof(TaskNodeHeader));

        // Execute the leaf nodes
        for (uint64_t i = 0; i < leafCount; i++)
        {
            auto leaf = leaves[i];
            if (mExecutedTasks.find(leaf) == mExecutedTasks.end())
            {
                auto leafNode = mGraph[leaf];
                ExecuteNode(leafNode);
            }
        }

        // Execute the task
        auto& task = mTasks[header->TaskHash];
        task.Execute(task.GetData());
        mExecutedTasks.emplace(header->TaskHash);
    }

private:
    /// @brief The tasks
    std::unordered_map<uint64_t, Task> mTasks;

    /// @brief The graph
    std::unordered_map<uint64_t, char*> mGraph;

    /// @brief The Top Nodes that have dependencies
    std::vector<char*> mTopNodes;

    /// @brief The executed tasks
    std::unordered_set<uint64_t> mExecutedTasks;
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
