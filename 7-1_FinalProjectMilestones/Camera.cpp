///////////////////////////////////////////////////////////////////////////////
// camera.cpp
// ============
// Implements a first-person style camera for OpenGL navigation
//
//  AUTHOR: Jose Medina (based on LearnOpenGL implementation)
//  UPDATED: Added ground-locked movement, dynamic speed control via mouse wheel,
//           smoother zoom, and improved vertical handling.
///////////////////////////////////////////////////////////////////////////////

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm> // for std::min and std::max

// ===== Constructor (vector-based) =====
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// ===== Alternate Constructor (explicit floats) =====
Camera::Camera(float posX, float posY, float posZ,
    float upX, float upY, float upZ,
    float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// ===== GetViewMatrix =====
glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

// ===== ProcessKeyboard =====
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    // Prevent sudden large frame spikes
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    float velocity = MovementSpeed * deltaTime;

    // Move along the ground (XZ plane) for forward/back
    if (direction == FORWARD)
        Position += glm::normalize(glm::vec3(Front.x, 0.0f, Front.z)) * velocity;
    if (direction == BACKWARD)
        Position -= glm::normalize(glm::vec3(Front.x, 0.0f, Front.z)) * velocity;

    // Strafe left/right
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;

    // Vertical motion (global up/down)
    if (direction == UP)
        Position += WorldUp * velocity;
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

// ===== ProcessMouseMovement =====
void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    // Clamp pitch to prevent flipping (manual version)
    if (constrainPitch)
    {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    updateCameraVectors();
}

// ===== ProcessMouseScroll =====
void Camera::ProcessMouseScroll(float yOffset)
{
    // Adjust FOV (zoom)
    Zoom -= yOffset * 2.0f;
    if (Zoom < 20.0f) Zoom = 20.0f;
    if (Zoom > 90.0f) Zoom = 90.0f;

    // Adjust movement speed dynamically with scroll wheel
    MovementSpeed += yOffset * 0.25f;
    if (MovementSpeed < 0.5f) MovementSpeed = 0.5f;
    if (MovementSpeed > 15.0f) MovementSpeed = 15.0f;
}

// ===== UpdateCameraVectors =====
void Camera::updateCameraVectors()
{
    // Calculate the new Front vector from yaw and pitch
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    // Recalculate Right and Up vectors
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
