///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// Manage the loading and rendering of 3D scenes
//
//  AUTHOR: Jose Medina
//  FINAL PROJECT - Milestone Four: Texturing + Materials + Composition
//  (2025) Adds texture loading/binding, clean per-object toggles, and comments
//  that map directly to the grading rubric.
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <iostream>
#include <string>

// ===== Shader uniform names (kept consistent with shaders) =====
namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";   // sampler2D binding
    const char* g_UseTextureName = "bUseTexture";     // bool/int
    const char* g_UseLightingName = "bUseLighting";    // bool/int
}

// Small helper
static inline glm::vec3 V3(float x, float y, float z) { return glm::vec3(x, y, z); }

// ============================================================================
//  Constructor / Destructor
// ============================================================================
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();
    m_loadedTextures = 0;
}

SceneManager::~SceneManager()
{
    m_pShaderManager = nullptr;
    delete m_basicMeshes;
    m_basicMeshes = nullptr;
}

// ============================================================================
//  Internal helpers
// ============================================================================
void SceneManager::SetTransformations(glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 model =
        glm::translate(positionXYZ) *
        glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1, 0, 0)) *
        glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0, 1, 0)) *
        glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0, 0, 1)) *
        glm::scale(scaleXYZ);

    if (m_pShaderManager)
        m_pShaderManager->setMat4Value(g_ModelName, model);
}

void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    if (!m_pShaderManager) return;

    m_pShaderManager->setIntValue(g_UseTextureName, false);
    m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(r, g, b, a));
    m_pShaderManager->setIntValue(g_UseLightingName, true);
}

void SceneManager::SetShaderMaterial(std::string materialTag)
{
    OBJECT_MATERIAL material{};
    for (auto& m : m_objectMaterials)
    {
        if (m.tag == materialTag) { material = m; break; }
    }

    if (!m_pShaderManager) return;

    m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
    m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
    m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
    m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
    m_pShaderManager->setFloatValue("material.shininess", material.shininess);
}

// Return an OpenGL texture id by its tag, or 0 if not found
GLuint SceneManager::FindTextureIDByTag(const std::string& tag) const
{
    for (int i = 0; i < m_loadedTextures; ++i)
        if (m_textureIDs[i].tag == tag) return m_textureIDs[i].ID;
    return 0;
}

// Toggle and bind a texture by tag on a specific unit (unit = 0..N)
void SceneManager::UseTextureByTag(const std::string& tag, int unit)
{
    if (!m_pShaderManager) return;

    GLuint tex = FindTextureIDByTag(tag);
    if (tex != 0)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);
        m_pShaderManager->setIntValue(g_UseLightingName, true);
        m_pShaderManager->setIntValue(g_TextureValueName, unit);

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    else
    {
        // Fallback to solid color if tag not found
        m_pShaderManager->setIntValue(g_UseTextureName, false);
    }
}

// ============================================================================
//  Texture Management
// ============================================================================
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0, height = 0, channels = 0;
    GLuint textureID = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);

    if (!image)
    {
        std::cout << "Could not load image: " << filename << std::endl;
        return false;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Reasonable defaults for most surfaces
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (channels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_textureIDs[m_loadedTextures].ID = textureID;
    m_textureIDs[m_loadedTextures].tag = tag;
    m_loadedTextures++;
    return true;
}

void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
        glDeleteTextures(1, &m_textureIDs[i].ID);
    m_loadedTextures = 0;
}

// ============================================================================
//  Prepare Scene (load meshes, materials, textures)
// ============================================================================
void SceneManager::PrepareScene()
{
    // Meshes with positions, normals, and UVs
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadTorusMesh();

    // Materials (lighting properties)
    m_objectMaterials.clear();
    OBJECT_MATERIAL matteWhite{ 0.20f, V3(1,1,1),       V3(1,1,1),        V3(0.2f,0.2f,0.2f),  4.0f,  "matteWhite" };
    OBJECT_MATERIAL plastic{ 0.15f, V3(0.8f,0.8f,0.8f), V3(0.8f,0.8f,0.8f),  V3(0.3f,0.3f,0.3f),  8.0f,  "plastic" };
    OBJECT_MATERIAL metal{ 0.10f, V3(0.9f,0.9f,0.9f), V3(0.8f,0.8f,0.85f), V3(1.0f,1.0f,1.0f), 32.0f, "metal" };
    m_objectMaterials.push_back(matteWhite);
    m_objectMaterials.push_back(plastic);
    m_objectMaterials.push_back(metal);

    // === Textures for Milestone Four ===
    // (1) Tile floor, (2) Rustic wood for the box, (3) Seamless gold for metallic pieces
    CreateGLTexture("../../Utilities/textures/pavers.jpg", "tex_floor");
    CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "tex_wood");
    CreateGLTexture("../../Utilities/textures/gold-seamless-texture.jpg", "tex_gold");

    // Optional: pre-bind to warm caches (not required)
    BindGLTextures();
}

// ============================================================================
//  Render Scene
// ============================================================================
void SceneManager::RenderScene()
{
    // ------------------------------------------------------------------------
    // 1) Ground plane — textured (pavers)
    // ------------------------------------------------------------------------
    SetShaderMaterial("matteWhite");
    UseTextureByTag("tex_floor", 0);                    // ✅ texture ON (unit 0)
    SetTransformations(glm::vec3(22.0f, 1.0f, 14.0f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(0.0f, -0.5f, 0.0f));
    m_basicMeshes->DrawPlaneMesh();

    // ------------------------------------------------------------------------
    // 2) Red Cylinder (Excedrin bottle body) — solid plastic (no texture)
    // ------------------------------------------------------------------------
    SetShaderMaterial("plastic");
    SetShaderColor(0.90f, 0.15f, 0.15f, 1.0f);         // ✅ texture OFF
    SetTransformations(glm::vec3(1.6f, 2.04f, 1.6f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(-6.0f, 1.05f, -1.0f));
    m_basicMeshes->DrawCylinderMesh();

    // Cap assembly (multi-shape cohesive object)
    // ------------------------------------------------------------------------
    // Gold base (bottom rim) — textured gold
    SetShaderMaterial("metal");
    UseTextureByTag("tex_gold", 2);
    SetTransformations(glm::vec3(1.68f, 0.21f, 1.68f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(-6.0f, 3.29f, -1.0f));
    m_basicMeshes->DrawCylinderMesh();

    // Red cap side walls — solid plastic
    SetShaderMaterial("plastic");
    SetShaderColor(0.90f, 0.10f, 0.10f, 1.0f);
    SetTransformations(glm::vec3(1.70f, 0.50f, 1.70f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(-6.0f, 3.55f, -1.0f));
    m_basicMeshes->DrawCylinderMesh();

    // Flat sealing lid (gold) — textured
    SetShaderMaterial("metal");
    UseTextureByTag("tex_gold", 2);
    SetTransformations(glm::vec3(1.60f, 0.45f, 1.60f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(-6.0f, 3.40f, -1.0f));
    m_basicMeshes->DrawCylinderMesh();

    // Decorative edge ring (gold) — torus, textured
    SetShaderMaterial("metal");
    UseTextureByTag("tex_gold", 2);
    SetTransformations(glm::vec3(1.65f, 0.80f, 1.65f),
        0.0f, 90.0f, 0.0f,
        glm::vec3(-6.0f, 3.97f, -1.0f));
    m_basicMeshes->DrawTorusMesh();

    // ------------------------------------------------------------------------
    // 3) WeGrower Box — textured wood (scaled 25% smaller previously)
    // ------------------------------------------------------------------------
    SetShaderMaterial("plastic");
    UseTextureByTag("tex_wood", 1);                    // wood texture on
    // Use neutral color when textured to avoid tinting
    m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(1, 1, 1, 1));
    SetTransformations(glm::vec3(2.625f, 1.5f, 1.875f),
        0.0f, 25.0f, 0.0f,
        glm::vec3(-1.0f, 0.75f, 2.0f));
    m_basicMeshes->DrawBoxMesh();

    // ------------------------------------------------------------------------
    // 4) Metal gray torus (washer) — solid color metal (no texture)
    // ------------------------------------------------------------------------
    SetShaderMaterial("metal");
    SetShaderColor(0.45f, 0.45f, 0.48f, 1.0f);
    SetTransformations(glm::vec3(0.9f, 0.9f, 0.2f),
        90.0f, 0.0f, 0.0f,
        glm::vec3(1.5f, 1.0f, -5.0f));
    m_basicMeshes->DrawTorusMesh();

    // ------------------------------------------------------------------------
    // 5) USB stick — plastic + metal (no textures, lit)
    // ------------------------------------------------------------------------
    SetShaderMaterial("plastic");
    SetShaderColor(0.25f, 0.25f, 0.30f, 1.0f);
    SetTransformations(glm::vec3(2.2f, 0.4f, 0.8f),
        0.0f, -35.0f, 0.0f,
        glm::vec3(3.0f, 0.7f, -2.0f));
    m_basicMeshes->DrawBoxMesh();

    SetShaderMaterial("metal");
    SetShaderColor(0.80f, 0.80f, 0.85f, 1.0f);
    SetTransformations(glm::vec3(0.7f, 0.35f, 0.7f),
        0.0f, -35.0f, 0.0f,
        glm::vec3(4.35f, 0.65f, -2.9f));
    m_basicMeshes->DrawBoxMesh();
}
