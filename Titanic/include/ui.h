/*
 * ui.h
 * Presentation layer header - declares all GUI rendering functions.
 * Uses Dear ImGui. Communicates only with the logic layer.
 */

#pragma once

// Initializes the UI state (call once at startup)
void uiInit();

// Renders the main application window (call every frame)
void uiRender();

// Cleans up UI state (call on shutdown)
void uiShutdown();
