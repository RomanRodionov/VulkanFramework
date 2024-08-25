#include "load_model.hpp"
#include <array>

void Model::processNode(aiNode *node, const aiScene *scene, aiMatrix4x4 transform)
{
    transform *= node->mTransformation;
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, transform));
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene, transform);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 transform)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    //std::vector<Texture2D> textures;

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;

        vertex.pos.x = mesh->mVertices[i].x;
        vertex.pos.y = mesh->mVertices[i].y;
        vertex.pos.z = mesh->mVertices[i].z;

        //to-do
        //vertex.normal.x = mesh->mNormals[i].x;
        //vertex.normal.y = mesh->mNormals[i].y;
        //vertex.normal.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0])
        {
            vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.texCoord = glm::vec2(0.f, 0.f);
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        auto face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

/* to-do
    if (mesh->mMaterialIndex >= 0)
    {
        auto *material = scene->mMaterials[mesh->mMaterialIndex];
        for (auto it = texTypesImage.begin(); it != texTypesImage.end(); it++)
        {
            loadMatTextures(textures, scene, material, it->first, it->second);
        }
    }
*/
    return Mesh(vertices, indices);
}

void VulkanApp::loadModel()
{
    Model model(MODEL_PATH.c_str());
    vertices = model.meshes[0].vertices;
    indices = model.meshes[0].indices;
}