/*
 * data.cpp
 * Data layer implementation.
 * Manages the in-memory task list and file persistence.
 */

#include "data.h"
#include <fstream>
#include <sstream>
#include <iostream>

// ---- Internal state ----

static std::vector<Task> taskList;  // In-memory task storage
static int nextId = 1;              // Auto-increment ID counter

// ---- Function implementations ----

std::vector<Task>& getAllTasks() {
    return taskList;
}

int addTask(const Task& task) {
    Task newTask  = task;
    newTask.id    = nextId++;
    taskList.push_back(newTask);
    return newTask.id;
}

bool removeTask(int id) {
    for (auto it = taskList.begin(); it != taskList.end(); ++it) {
        if (it->id == id) {
            taskList.erase(it);
            return true;
        }
    }
    return false;
}

bool updateTask(const Task& updatedTask) {
    for (auto& task : taskList) {
        if (task.id == updatedTask.id) {
            task = updatedTask;
            return true;
        }
    }
    return false;
}

int getNextId() {
    return nextId;
}

std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::HIGH:   return "HIGH";
        case Priority::MEDIUM: return "MEDIUM";
        case Priority::LOW:    return "LOW";
        default:               return "LOW";
    }
}

Priority stringToPriority(const std::string& s) {
    if (s == "HIGH")   return Priority::HIGH;
    if (s == "MEDIUM") return Priority::MEDIUM;
    return Priority::LOW;
}

void saveTasksToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file for writing: " << filename << std::endl;
        return;
    }

    // Write next ID on the first line
    file << nextId << "\n";

    for (const auto& task : taskList) {
        file << task.id          << "|"
             << task.title       << "|"
             << task.description << "|"
             << priorityToString(task.priority) << "|"
             << task.deadline    << "|"
             << task.duration    << "|"
             << (task.completed ? 1 : 0) << "\n";
    }
}

void loadTasksFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return; // File doesn't exist yet - that's OK
    }

    taskList.clear();

    std::string line;
    // Read next ID
    if (std::getline(file, line)) {
        nextId = std::stoi(line);
    }

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        Task task;

        // Parse pipe-separated fields
        std::getline(ss, token, '|'); task.id          = std::stoi(token);
        std::getline(ss, token, '|'); task.title        = token;
        std::getline(ss, token, '|'); task.description  = token;
        std::getline(ss, token, '|'); task.priority     = stringToPriority(token);
        std::getline(ss, token, '|'); task.deadline     = token;
        std::getline(ss, token, '|'); task.duration     = std::stoi(token);
        std::getline(ss, token, '|'); task.completed    = (token == "1");

        taskList.push_back(task);
    }
}
