/*
 * ui.cpp
 * Presentation layer implementation.
 * Renders all ImGui windows. Calls only logic layer functions.
 * Does NOT access data layer directly.
 */

#include "ui.h"
#include "logic.h"
#include "data.h"

#include "imgui.h"
#include <string>
#include <vector>
#include <cstring>

// ---- UI State ----

// Add Task form fields
static char  newTitle[128]       = "";
static char  newDescription[256] = "";
static char  newDeadline[16]     = "2025-12-31";
static int   newDuration         = 30;
static int   newPriorityIndex    = 1; // 0=LOW, 1=MEDIUM, 2=HIGH

// Search field
static char  searchQuery[128] = "";

// Filter state
static int   filterMode = 0; // 0=All, 1=Priority, 2=Deadline, 3=Completed, 4=Pending

// Error / feedback messages
static std::string feedbackMsg  = "";
static bool        feedbackIsOk = true;

// Stats window toggle
static bool showStats = false;

// Edit task state
static bool editMode    = false;
static Task editingTask = {};
static char editTitle[128]       = "";
static char editDescription[256] = "";
static char editDeadline[16]     = "";
static int  editDuration         = 0;
static int  editPriorityIndex    = 0;

// Priority label helper
static const char* PRIORITY_LABELS[] = { "LOW", "MEDIUM", "HIGH" };

static Priority indexToPriority(int idx) {
    switch (idx) {
        case 0: return Priority::LOW;
        case 1: return Priority::MEDIUM;
        case 2: return Priority::HIGH;
        default: return Priority::MEDIUM;
    }
}

static int priorityToIndex(Priority p) {
    switch (p) {
        case Priority::LOW:    return 0;
        case Priority::MEDIUM: return 1;
        case Priority::HIGH:   return 2;
        default:               return 1;
    }
}

// Priority color helper
static ImVec4 priorityColor(Priority p) {
    switch (p) {
        case Priority::HIGH:   return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red
        case Priority::MEDIUM: return ImVec4(1.0f, 0.8f, 0.1f, 1.0f); // Yellow
        case Priority::LOW:    return ImVec4(0.4f, 0.9f, 0.4f, 1.0f); // Green
        default:               return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

// ---- UI Sections ----

static void renderAddTaskPanel() {
    ImGui::SeparatorText("Add New Task");

    ImGui::InputText("Title##add",       newTitle,       sizeof(newTitle));
    ImGui::InputText("Description##add", newDescription, sizeof(newDescription));
    ImGui::InputText("Deadline (YYYY-MM-DD)##add", newDeadline, sizeof(newDeadline));
    ImGui::InputInt("Duration (minutes)##add", &newDuration);
    ImGui::Combo("Priority##add", &newPriorityIndex, PRIORITY_LABELS, 3);

    if (ImGui::Button("Add Task", ImVec2(120, 0))) {
        int id = createTask(
            std::string(newTitle),
            std::string(newDescription),
            indexToPriority(newPriorityIndex),
            std::string(newDeadline),
            newDuration
        );

        if (id > 0) {
            feedbackMsg  = "Task added successfully! (ID: " + std::to_string(id) + ")";
            feedbackIsOk = true;
            saveTasksToFile("tasks.dat");
            // Clear form
            newTitle[0]       = '\0';
            newDescription[0] = '\0';
            newDuration       = 30;
            newPriorityIndex  = 1;
        } else {
            feedbackMsg  = "Error: Invalid task data. Check all fields.";
            feedbackIsOk = false;
        }
    }
}

static void renderSearchPanel() {
    ImGui::SeparatorText("Search & Filter");

    ImGui::InputText("Search by title", searchQuery, sizeof(searchQuery));

    ImGui::RadioButton("All",       &filterMode, 0); ImGui::SameLine();
    ImGui::RadioButton("Priority",  &filterMode, 1); ImGui::SameLine();
    ImGui::RadioButton("Deadline",  &filterMode, 2); ImGui::SameLine();
    ImGui::RadioButton("Completed", &filterMode, 3); ImGui::SameLine();
    ImGui::RadioButton("Pending",   &filterMode, 4);
}

static void renderEditModal() {
    if (!editMode) return;

    ImGui::OpenPopup("Edit Task");
    if (ImGui::BeginPopupModal("Edit Task", &editMode, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Title##edit",       editTitle,       sizeof(editTitle));
        ImGui::InputText("Description##edit", editDescription, sizeof(editDescription));
        ImGui::InputText("Deadline##edit",    editDeadline,    sizeof(editDeadline));
        ImGui::InputInt("Duration##edit",    &editDuration);
        ImGui::Combo("Priority##edit",       &editPriorityIndex, PRIORITY_LABELS, 3);

        if (ImGui::Button("Save", ImVec2(80, 0))) {
            editingTask.title       = std::string(editTitle);
            editingTask.description = std::string(editDescription);
            editingTask.deadline    = std::string(editDeadline);
            editingTask.duration    = editDuration;
            editingTask.priority    = indexToPriority(editPriorityIndex);

            std::string err = validateTask(editingTask);
            if (err.empty()) {
                updateTask(editingTask);
                saveTasksToFile("tasks.dat");
                feedbackMsg  = "Task updated successfully!";
                feedbackIsOk = true;
                editMode     = false;
                ImGui::CloseCurrentPopup();
            } else {
                feedbackMsg  = "Error: " + err;
                feedbackIsOk = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            editMode = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void renderTaskTable() {
    ImGui::SeparatorText("Task List");

    // Build the task list based on filter and search
    std::vector<Task> tasks;
    std::string query(searchQuery);

    if (!query.empty()) {
        tasks = searchByTitle(query);
    } else {
        switch (filterMode) {
            case 1: tasks = getTasksSortedByPriority(); break;
            case 2: tasks = getTasksSortedByDeadline(); break;
            case 3: tasks = filterByStatus(true);        break;
            case 4: tasks = filterByStatus(false);       break;
            default: tasks = getAllTasks();               break;
        }
    }

    // Table
    ImGuiTableFlags flags = ImGuiTableFlags_Borders
                          | ImGuiTableFlags_RowBg
                          | ImGuiTableFlags_ScrollY
                          | ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("TaskTable", 7, flags, ImVec2(0, 320))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID",          ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Title",       ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Priority",    ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Deadline",    ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Duration",    ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Status",      ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Actions",     ImGuiTableColumnFlags_WidthFixed, 140);
        ImGui::TableHeadersRow();

        for (const auto& task : tasks) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", task.id);

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(task.title.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextColored(priorityColor(task.priority),
                               "%s", priorityToString(task.priority).c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(task.deadline.empty() ? "-" : task.deadline.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d min", task.duration);

            ImGui::TableSetColumnIndex(5);
            if (task.completed) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Done");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.1f, 1.0f), "Pending");
            }

            ImGui::TableSetColumnIndex(6);
            // Complete button
            if (!task.completed) {
                ImGui::PushID(task.id * 10 + 1);
                if (ImGui::SmallButton("Complete")) {
                    completeTask(task.id);
                    saveTasksToFile("tasks.dat");
                    feedbackMsg  = "Task marked as completed.";
                    feedbackIsOk = true;
                }
                ImGui::PopID();
                ImGui::SameLine();
            }

            // Edit button
            ImGui::PushID(task.id * 10 + 2);
            if (ImGui::SmallButton("Edit")) {
                editingTask      = task;
                editMode         = true;
                editPriorityIndex = priorityToIndex(task.priority);
                editDuration     = task.duration;
                std::strncpy(editTitle,       task.title.c_str(),       sizeof(editTitle) - 1);
                std::strncpy(editDescription, task.description.c_str(), sizeof(editDescription) - 1);
                std::strncpy(editDeadline,    task.deadline.c_str(),    sizeof(editDeadline) - 1);
            }
            ImGui::PopID();
            ImGui::SameLine();

            // Delete button
            ImGui::PushID(task.id * 10 + 3);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            if (ImGui::SmallButton("Delete")) {
                deleteTask(task.id);
                saveTasksToFile("tasks.dat");
                feedbackMsg  = "Task deleted.";
                feedbackIsOk = true;
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

static void renderStatsWindow() {
    if (!showStats) return;

    ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_Once);
    if (ImGui::Begin("Statistics", &showStats)) {
        const std::vector<Task>& all = getAllTasks();

        int total     = static_cast<int>(all.size());
        int completed = static_cast<int>(filterByStatus(true).size());
        int pending   = total - completed;
        int highCount = countHighPriorityRecursive(all);  // Recursive!
        int totalMin  = totalDurationRecursive(all);       // Recursive!

        ImGui::SeparatorText("Overview");
        ImGui::Text("Total tasks    : %d", total);
        ImGui::Text("Completed      : %d", completed);
        ImGui::Text("Pending        : %d", pending);

        ImGui::SeparatorText("Recursive Calculations");
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                           "HIGH priority tasks: %d", highCount);
        ImGui::Text("Total duration : %d min (%d h %d min)",
                    totalMin, totalMin / 60, totalMin % 60);

        ImGui::SeparatorText("Progress");
        if (total > 0) {
            float ratio = static_cast<float>(completed) / static_cast<float>(total);
            ImGui::ProgressBar(ratio, ImVec2(-1, 0));
            ImGui::Text("%.0f%% complete", ratio * 100.0f);
        } else {
            ImGui::TextDisabled("No tasks yet.");
        }
    }
    ImGui::End();
}

// ---- Public API ----

void uiInit() {
    loadTasksFromFile("tasks.dat");
}

void uiRender() {
    // Full-screen main window
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Titanic - Task Manager",
                 nullptr,
                 ImGuiWindowFlags_NoResize    |
                 ImGuiWindowFlags_NoMove      |
                 ImGuiWindowFlags_NoCollapse  |
                 ImGuiWindowFlags_MenuBar);

    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save")) {
                saveTasksToFile("tasks.dat");
                feedbackMsg  = "Tasks saved to tasks.dat";
                feedbackIsOk = true;
            }
            if (ImGui::MenuItem("Reload")) {
                loadTasksFromFile("tasks.dat");
                feedbackMsg  = "Tasks reloaded.";
                feedbackIsOk = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Statistics", nullptr, &showStats);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Feedback message
    if (!feedbackMsg.empty()) {
        if (feedbackIsOk) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "%s", feedbackMsg.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", feedbackMsg.c_str());
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("x")) {
            feedbackMsg = "";
        }
    }

    renderAddTaskPanel();
    renderSearchPanel();
    renderTaskTable();
    renderEditModal();

    ImGui::End();

    renderStatsWindow();
}

void uiShutdown() {
    saveTasksToFile("tasks.dat");
}
