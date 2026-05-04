/*
 * logic.cpp
 * Logic layer implementation.
 * Contains all business rules, sorting, searching and recursive algorithms.
 */

#include "logic.h"
#include "data.h"
#include <algorithm>
#include <cctype>
#include <sstream>

// ---- Sorting algorithms ----

// Bubble Sort by priority (HIGH = 3, MEDIUM = 2, LOW = 1) - descending
void sortByPriority(std::vector<Task>& tasks) {
    int n = static_cast<int>(tasks.size());
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (static_cast<int>(tasks[j].priority) <
                static_cast<int>(tasks[j + 1].priority)) {
                std::swap(tasks[j], tasks[j + 1]);
            }
        }
    }
}

// Bubble Sort by deadline string (lexicographic - works for YYYY-MM-DD format)
void sortByDeadline(std::vector<Task>& tasks) {
    int n = static_cast<int>(tasks.size());
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (tasks[j].deadline > tasks[j + 1].deadline) {
                std::swap(tasks[j], tasks[j + 1]);
            }
        }
    }
}

// ---- Searching algorithms ----

// Helper: converts a string to lowercase
static std::string toLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(c));
    }
    return result;
}

// Linear search by title substring (case-insensitive)
std::vector<Task> searchByTitle(const std::string& query) {
    std::vector<Task> results;
    std::string lowerQuery = toLower(query);

    for (const auto& task : getAllTasks()) {
        if (toLower(task.title).find(lowerQuery) != std::string::npos) {
            results.push_back(task);
        }
    }
    return results;
}

std::vector<Task> filterByPriority(Priority p) {
    std::vector<Task> results;
    for (const auto& task : getAllTasks()) {
        if (task.priority == p) {
            results.push_back(task);
        }
    }
    return results;
}

std::vector<Task> filterByStatus(bool completed) {
    std::vector<Task> results;
    for (const auto& task : getAllTasks()) {
        if (task.completed == completed) {
            results.push_back(task);
        }
    }
    return results;
}

// ---- Recursive algorithms ----

// Recursively sums duration of all tasks starting from index
int totalDurationRecursive(const std::vector<Task>& tasks, int index) {
    if (index >= static_cast<int>(tasks.size())) {
        return 0; // Base case: no more tasks
    }
    return tasks[index].duration + totalDurationRecursive(tasks, index + 1);
}

// Recursively counts HIGH priority tasks starting from index
int countHighPriorityRecursive(const std::vector<Task>& tasks, int index) {
    if (index >= static_cast<int>(tasks.size())) {
        return 0; // Base case
    }
    int current = (tasks[index].priority == Priority::HIGH) ? 1 : 0;
    return current + countHighPriorityRecursive(tasks, index + 1);
}

// ---- Task management ----

std::string validateTask(const Task& task) {
    if (task.title.empty()) {
        return "Title cannot be empty.";
    }
    if (task.title.length() > 100) {
        return "Title is too long (max 100 characters).";
    }
    if (task.duration <= 0) {
        return "Duration must be a positive number.";
    }
    if (task.duration > 10000) {
        return "Duration is too large (max 10000 minutes).";
    }
    // Validate deadline format YYYY-MM-DD
    if (!task.deadline.empty() && task.deadline.length() != 10) {
        return "Deadline must be in YYYY-MM-DD format.";
    }
    return ""; // No errors
}

int createTask(const std::string& title,
               const std::string& description,
               Priority           priority,
               const std::string& deadline,
               int                duration)
{
    Task task;
    task.id          = 0; // Will be assigned by data layer
    task.title       = title;
    task.description = description;
    task.priority    = priority;
    task.deadline    = deadline;
    task.duration    = duration;
    task.completed   = false;

    std::string error = validateTask(task);
    if (!error.empty()) {
        return -1; // Validation failed
    }

    return addTask(task);
}

bool completeTask(int id) {
    for (auto& task : getAllTasks()) {
        if (task.id == id) {
            task.completed = true;
            return true;
        }
    }
    return false;
}

bool deleteTask(int id) {
    return removeTask(id);
}

std::vector<Task> getTasksSortedByPriority() {
    std::vector<Task> tasks = getAllTasks();
    sortByPriority(tasks);
    return tasks;
}

std::vector<Task> getTasksSortedByDeadline() {
    std::vector<Task> tasks = getAllTasks();
    sortByDeadline(tasks);
    return tasks;
}
