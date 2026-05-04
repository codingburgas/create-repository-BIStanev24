/*
 * data.h
 * Data layer header - defines core structures and data access functions.
 * Responsible for storing and retrieving task data.
 */

#pragma once
#include <string>
#include <vector>

// Priority levels for tasks
enum class Priority {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3
};

// Task structure - core data unit
struct Task {
    int         id;
    std::string title;
    std::string description;
    Priority    priority;
    std::string deadline;    // Format: "YYYY-MM-DD"
    int         duration;    // Estimated duration in minutes
    bool        completed;
};

// ---- Data layer functions ----

// Returns a reference to the global task list
std::vector<Task>& getAllTasks();

// Adds a task to the list and returns its assigned ID
int  addTask(const Task& task);

// Removes a task by ID; returns true if found and removed
bool removeTask(int id);

// Updates an existing task; returns true if found
bool updateTask(const Task& updatedTask);

// Returns the next available unique ID
int  getNextId();

// Saves tasks to a file
void saveTasksToFile(const std::string& filename);

// Loads tasks from a file
void loadTasksFromFile(const std::string& filename);

// Converts Priority enum to a readable string
std::string priorityToString(Priority p);

// Converts a string to Priority enum
Priority stringToPriority(const std::string& s);
