#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "vulkan_app.hpp"
#include "vertex_data.hpp"
#include <map>
#include <filesystem>

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from);

class Mesh
{
public:
    glm::mat4x4 m_transform;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Mesh(std::vector<Vertex>& vertices_, std::vector<uint32_t>& indices_): 
        vertices(std::move(vertices_)), indices(std::move(indices_))
    {
        //setup();
        m_transform = glm::mat4x4(1.f);
    }
};

class Model
{
private:

    void load(const char* path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error(std::string("ERROR::ASSIMP::") + std::string(importer.GetErrorString()));
            return;
        }
        directory = std::filesystem::path(path).parent_path().string();

        
        int upAxis = 2;
        scene->mMetaData->Get<int>("UpAxis", upAxis);
        int upAxisSign = 1;
        scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
        int frontAxis = 1;
        scene->mMetaData->Get<int>("FrontAxis", frontAxis);
        int frontAxisSign = 1;
        scene->mMetaData->Get<int>("FrontAxisSign", frontAxisSign);
        int coordAxis = 0;
        scene->mMetaData->Get<int>("CoordAxis", coordAxis);
        int coordAxisSign = 1;
        scene->mMetaData->Get<int>("CoordAxisSign", coordAxisSign);
        float scaleFactor;
        scene->mMetaData->Get<float>("UnitScaleFactor", scaleFactor);

        aiVector3D upVec = upAxis == 0 ? aiVector3D(upAxisSign,0,0) : upAxis == 1 ? aiVector3D(0, upAxisSign,0) : aiVector3D(0, 0, upAxisSign);
        aiVector3D forwardVec = frontAxis == 0 ? aiVector3D(frontAxisSign, 0, 0) : frontAxis == 1 ? aiVector3D(0, frontAxisSign, 0) : aiVector3D(0, 0, frontAxisSign);
        aiVector3D rightVec = coordAxis == 0 ? aiVector3D(coordAxisSign, 0, 0) : coordAxis == 1 ? aiVector3D(0, coordAxisSign, 0) : aiVector3D(0, 0, coordAxisSign);
        aiMatrix4x4 transMat(rightVec.x, rightVec.y, rightVec.z, 0.f,
                        upVec.x, upVec.y, upVec.z, 0.f,
                        forwardVec.x, forwardVec.y, forwardVec.z, 0.f,
                        0.f, 0.f, 0.f, 1.f);

        transMat *= aiMatrix4x4(1 / scaleFactor, 0.f, 0.f, 0.f,
                                0.f, 1 / scaleFactor, 0.f, 0.f,
                                0.f, 0.f, 1 / scaleFactor, 0.f,
                                0.f, 0.f, 0.f, 1.f);
        
        scene->mRootNode->mTransformation *= transMat;
        
        processNode(scene->mRootNode, scene, scene->mRootNode->mTransformation);
    }
    void processNode(aiNode *node, const aiScene *scene, aiMatrix4x4 transform);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 transform);
public:
    std::vector<Mesh> meshes;
    std::string directory;

    Model(const char *path)
    {
        load(path);
    }
};