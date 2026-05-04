/*
 * logic.h
 * Logic layer header - declares all business logic functions.
 * Acts as the bridge between UI and data layers.
 */

#pragma once
#include "data.h"
#include <vector>
#include <string>

// ---- Sorting ----

// Sorts tasks by priority (HIGH first)
void sortByPriority(std::vector<Task>& tasks);

// Sorts tasks by deadline (earliest first)
void sortByDeadline(std::vector<Task>& tasks);

// ---- Searching ----

// Returns tasks whose title contains the search string (case-insensitive)
std::vector<Task> searchByTitle(const std::string& query);

// Returns tasks matching the given priority
std::vector<Task> filterByPriority(Priority p);

// Returns tasks that are completed or not completed
std::vector<Task> filterByStatus(bool completed);

// ---- Recursive operations ----

// Recursively calculates total duration of all tasks in the list (minutes)
int totalDurationRecursive(const std::vector<Task>& tasks, int index = 0);

// Recursively counts tasks with HIGH priority
int countHighPriorityRecursive(const std::vector<Task>& tasks, int index = 0);

// ---- Task management ----

// Validates task fields; returns an error message or empty string if valid
std::string validateTask(const Task& task);

// Creates and adds a new task; returns ID or -1 on validation failure
int  createTask(const std::string& title,
                const std::string& description,
                Priority           priority,
                const std::string& deadline,
                int                duration);

// Marks a task as completed by ID; returns true if found
bool completeTask(int id);

// Deletes a task by ID; returns true if found
bool deleteTask(int id);

// Returns a copy of all tasks sorted by priority
std::vector<Task> getTasksSortedByPriority();

// Returns a copy of all tasks sorted by deadline
std::vector<Task> getTasksSortedByDeadline();
