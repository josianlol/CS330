///////////////////////////////////////////////////////////////////////////////
// maincode.cpp
// ============
// Entry point for the 7-1 Final Project and Milestones
//
// AUTHOR: Jose Medina
// COURSE: CS-330 Computational Graphics and Visualization
// UPDATED (2025): Refactored event loop and context handling
///////////////////////////////////////////////////////////////////////////////

#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// ======= Global Namespace =======
namespace
{
    const char* const WINDOW_TITLE = "7-1 Final Project and Milestones";

    GLFWwindow* g_Window = nullptr;
    SceneManager* g_SceneManager = nullptr;
    ShaderManager* g_ShaderManager = nullptr;
    ViewManager* g_ViewManager = nullptr;
}

// ======= Function Declarations =======
bool InitializeGLFW();
bool InitializeGLEW();

// ======= main() =======
int main(int argc, char* argv[])
{
    // Initialize GLFW
    if (!InitializeGLFW())
        return EXIT_FAILURE;

    // Create Shader and View managers
    g_ShaderManager = new ShaderManager();
    g_ViewManager = new ViewManager(g_ShaderManager);

    // Create the display window
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);
    if (!g_Window)
        return EXIT_FAILURE;

    // Initialize GLEW
    if (!InitializeGLEW())
        return EXIT_FAILURE;

    // Load, compile, and link shaders
    g_ShaderManager->LoadShaders(
        "../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    // Prepare the 3D scene
    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    // Enable z-depth and blending globally
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // =======================
    // 🔁 Main Render Loop
    // =======================
    while (!glfwWindowShouldClose(g_Window))
    {
        // --- Input Handling ---
        glfwPollEvents(); // ✅ process all keyboard/mouse events first

        // --- Frame Setup ---
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Update View & Camera ---
        g_ViewManager->PrepareSceneView();

        // --- Render 3D Scene ---
        g_SceneManager->RenderScene();

        // --- Swap Buffers ---
        glfwSwapBuffers(g_Window);
    }

    // =======================
    // 🧹 Cleanup
    // =======================
    if (g_SceneManager) { delete g_SceneManager; g_SceneManager = nullptr; }
    if (g_ViewManager) { delete g_ViewManager;  g_ViewManager = nullptr; }
    if (g_ShaderManager) { delete g_ShaderManager;g_ShaderManager = nullptr; }

    glfwTerminate();
    return EXIT_SUCCESS;
}

// ======= InitializeGLFW() =======
bool InitializeGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return true;
}

// ======= InitializeGLEW() =======
bool InitializeGLEW()
{
    GLenum GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << "GLEW Initialization Error: " << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }

    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n\n";
    return true;
}
