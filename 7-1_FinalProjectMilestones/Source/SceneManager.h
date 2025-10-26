///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// Manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  UPDATED BY: Jose Medina - CS-330 Final Project (Milestone Four)
//  (2025) Added texture helpers and consistent modular structure
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

/***********************************************************
 *  SceneManager
 *
 *  Handles loading meshes, materials, and textures.
 *  Manages transformations, shader settings, and full
 *  object rendering for the 3D scene.
 ***********************************************************/
class SceneManager
{
public:
    // ===== Constructor & Destructor =====
    SceneManager(ShaderManager* pShaderManager);
    ~SceneManager();

    // ===== Structures =====
    struct TEXTURE_INFO
    {
        std::string tag;
        uint32_t ID;
    };

    struct OBJECT_MATERIAL
    {
        float ambientStrength;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
        std::string tag;
    };

private:
    // ===== Internal Pointers =====
    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;

    // ===== Texture Management =====
    int m_loadedTextures;
    TEXTURE_INFO m_textureIDs[16];

    // ===== Material Definitions =====
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    // ===== Private Utility Functions =====
    bool CreateGLTexture(const char* filename, std::string tag);
    void BindGLTextures();
    void DestroyGLTextures();

    // ===== Transformations & Shader Utilities =====
    void SetTransformations(
        glm::vec3 scaleXYZ,
        float XrotationDegrees,
        float YrotationDegrees,
        float ZrotationDegrees,
        glm::vec3 positionXYZ);

    void SetShaderColor(
        float redColorValue,
        float greenColorValue,
        float blueColorValue,
        float alphaValue);

    void SetShaderMaterial(std::string materialTag);

    // ===== Internal Texture Helpers =====
    GLuint FindTextureIDByTag(const std::string& tag) const;
    void UseTextureByTag(const std::string& tag, int unit);

public:
    /***********************************************************
     * Public Accessors & Wrappers
     ***********************************************************/
    ShapeMeshes* GetShapeMeshes() { return m_basicMeshes; }

    void PublicSetShaderColor(float r, float g, float b, float a)
    {
        SetShaderColor(r, g, b, a);
    }

    void PublicSetTransformations(
        glm::vec3 scaleXYZ,
        float XrotationDegrees,
        float YrotationDegrees,
        float ZrotationDegrees,
        glm::vec3 positionXYZ)
    {
        SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    }

    /***********************************************************
     * Scene Lifecycle Methods
     ***********************************************************/
    void PrepareScene();
    void RenderScene();
};

