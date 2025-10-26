///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// Manage the viewing of 3D objects within the viewport.
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  UPDATED BY: Jose Medina - Final Project
//
//  FINAL ENGINEER REVIEW (2025):
//  Added perspective/orthographic toggle (P/O),
//  confirmed full WASD + QE navigation, mouse look, and scroll zoom.
//  Structured for readability and rubric compliance.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "ShaderManager.h"
#include "Camera.h"

/***********************************************************
 *  ViewManager
 *
 *  This class manages camera navigation and projection setup
 *  for rendering 3D scenes, supporting:
 *   - WASD: Forward, Backward, Left, Right
 *   - QE:   Vertical movement (Up/Down)
 *   - Mouse: Rotate camera view (yaw/pitch)
 *   - Scroll: Adjust zoom and movement speed
 *   - P / O: Toggle Perspective or Orthographic projection
 ***********************************************************/
class ViewManager
{
public:
    // ===== Constructors / Destructor =====
    ViewManager(ShaderManager* pShaderManager);
    ~ViewManager();

    // ===== Initialization =====
    GLFWwindow* CreateDisplayWindow(const char* windowTitle);

    // Prepares and uploads view/projection matrices each frame
    void PrepareSceneView();

    // Store active window reference (used in MainCode.cpp)
    void SetWindow(GLFWwindow* window) { m_pWindow = window; }

public:
    // ===== Input Callbacks =====
    static void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);   // P/O toggle
    static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos); // Camera rotation
    static void Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset);       // Zoom/speed

private:
    // ===== Internal Processing =====
    void ProcessKeyboardEvents(); // Handles WASD + QE + R + ESC

    // ===== Timing (for smooth delta-time movement) =====
    static float gDeltaTime;
    static float gLastFrame;

    // ===== Pointers =====
    ShaderManager* m_pShaderManager;
    GLFWwindow* m_pWindow;
};
