///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// Manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  UPDATED BY: Jose Medina - Final Project (Lighting + Camera + Controls)
//
//  FINAL REVIEW (2025):
//  • Configured directional + point lighting
//  • Implemented WASD/QE + mouse/scroll navigation
//  • Added orthographic/perspective toggle (P/O)
//  • Tuned ambient light for balanced brightness
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ======= GLOBAL TIMING VARIABLES =======
float ViewManager::gDeltaTime = 0.0f;
float ViewManager::gLastFrame = 0.0f;

// ======= GLOBAL CAMERA VARIABLES =======
namespace
{
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    Camera* g_pCamera = nullptr;
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    bool bOrthographicProjection = false;
}

// ======= Constructor =======
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_pWindow = nullptr;

    // Initialize camera defaults
    g_pCamera = new Camera();
    g_pCamera->Position = glm::vec3(0.0f, 8.0f, 25.0f);
    g_pCamera->Yaw = -90.0f;
    g_pCamera->Pitch = -15.0f;
    g_pCamera->MovementSpeed = 10.0f;
    g_pCamera->updateCameraVectors();
}

// ======= Destructor =======
ViewManager::~ViewManager()
{
    m_pShaderManager = nullptr;
    m_pWindow = nullptr;
    if (g_pCamera)
    {
        delete g_pCamera;
        g_pCamera = nullptr;
    }
}

// ======= CreateDisplayWindow() =======
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwFocusWindow(window);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Register callbacks
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
    glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);
    glfwSetKeyCallback(window, &ViewManager::Key_Callback);

    // Enable blending (transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return window;
}

// ======= Mouse Position Callback =======
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    if (gFirstMouse)
    {
        gLastX = static_cast<float>(xMousePos);
        gLastY = static_cast<float>(yMousePos);
        gFirstMouse = false;
    }

    float xOffset = static_cast<float>(xMousePos) - gLastX;
    float yOffset = gLastY - static_cast<float>(yMousePos);
    gLastX = static_cast<float>(xMousePos);
    gLastY = static_cast<float>(yMousePos);

    if (g_pCamera)
        g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

// ======= Mouse Scroll Callback =======
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
    if (g_pCamera)
        g_pCamera->ProcessMouseScroll(static_cast<float>(yOffset));
}

// ======= Key Callback =======
void ViewManager::Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        // Toggle between perspective and orthographic projections
        if (key == GLFW_KEY_P)
        {
            bOrthographicProjection = false;
            std::cout << "Perspective view enabled.\n";
        }
        else if (key == GLFW_KEY_O)
        {
            bOrthographicProjection = true;
            std::cout << "Orthographic view enabled.\n";
        }
    }
}

// ======= ProcessKeyboardEvents() =======
void ViewManager::ProcessKeyboardEvents()
{
    if (!m_pWindow)
        return;

    float currentFrame = static_cast<float>(glfwGetTime());
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_pWindow, true);

    // === Camera Movement Controls ===
    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
        g_pCamera->ProcessKeyboard(UP, gDeltaTime);

    // Reset camera position
    if (glfwGetKey(m_pWindow, GLFW_KEY_R) == GLFW_PRESS)
    {
        g_pCamera->Position = glm::vec3(0.0f, 8.0f, 25.0f);
        g_pCamera->Yaw = -90.0f;
        g_pCamera->Pitch = -15.0f;
        g_pCamera->updateCameraVectors();
    }
}

// ======= PrepareSceneView() =======
void ViewManager::PrepareSceneView()
{
    // Update delta time
    float currentFrame = static_cast<float>(glfwGetTime());
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    // Handle user input
    ProcessKeyboardEvents();

    // Build view and projection matrices
    glm::mat4 view = g_pCamera->GetViewMatrix();
    glm::mat4 projection;

    if (!bOrthographicProjection)
    {
        projection = glm::perspective(glm::radians(g_pCamera->Zoom),
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 200.0f);
    }
    else
    {
        projection = glm::ortho(-10.0f, 10.0f, -8.0f, 8.0f, 0.1f, 100.0f);
    }

    // Send uniforms to shader
    if (m_pShaderManager)
    {
        m_pShaderManager->use();
        m_pShaderManager->setMat4Value(g_ViewName, view);
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);

        /***********************************************************
         * LIGHTING CONFIGURATION — Tuned for Final Project
         ***********************************************************/

         // === Light Source 1: Warm Key Light ===
        m_pShaderManager->setVec3Value("lightSources[0].position", glm::vec3(5.0f, 6.0f, 4.0f));
        m_pShaderManager->setVec3Value("lightSources[0].ambientColor", glm::vec3(0.35f, 0.3f, 0.25f)); // adjusted ambient
        m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(0.9f, 0.85f, 0.8f));
        m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(1.0f, 0.95f, 0.9f));
        m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 1.0f);

        // === Light Source 2: Cool Fill Light ===
        m_pShaderManager->setVec3Value("lightSources[1].position", glm::vec3(-6.0f, 5.0f, -3.0f));
        m_pShaderManager->setVec3Value("lightSources[1].ambientColor", glm::vec3(0.1f, 0.1f, 0.15f));
        m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", glm::vec3(0.5f, 0.55f, 0.7f));
        m_pShaderManager->setVec3Value("lightSources[1].specularColor", glm::vec3(0.6f, 0.6f, 0.8f));
        m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.8f);

        // === Directional Light (Sunlight) ===
        m_pShaderManager->setVec3Value("dirLight.direction", glm::vec3(-0.3f, -1.0f, -0.25f));
        m_pShaderManager->setVec3Value("dirLight.ambient", glm::vec3(0.35f, 0.35f, 0.35f));
        m_pShaderManager->setVec3Value("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        m_pShaderManager->setVec3Value("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // === Point Light (Warm Reflection Source) ===
        m_pShaderManager->setVec3Value("pointLight.position", glm::vec3(2.0f, 5.0f, 2.0f));
        m_pShaderManager->setVec3Value("pointLight.ambient", glm::vec3(0.4f, 0.35f, 0.25f));
        m_pShaderManager->setVec3Value("pointLight.diffuse", glm::vec3(1.0f, 0.9f, 0.75f));
        m_pShaderManager->setVec3Value("pointLight.specular", glm::vec3(1.0f, 0.95f, 0.9f));
    }
}
