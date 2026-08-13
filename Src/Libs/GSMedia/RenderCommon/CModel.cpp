#include "d3dUtil.h"
#include "CModel.h"
#include "TextureManager.h"
//#include "ImGuiLog.h"

#include <QString>
#include <QFileInfo>

#include <iostream>

#include <filesystem>
#include <codecvt>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>      // 通用 PBR 材质键
#include <assimp/GltfMaterial.h> // glTF 专用属性 (alphaMode, alphaCutoff)

#include "GeometryDefine.h"

#ifdef _DEBUG
#pragma comment(lib, "assimp-vc141-mtd.lib")
#else
#pragma comment(lib, "assimp-vc141-mt.lib")
#endif // _DEBUG

void CModel::Draw(ID3D11DeviceContext* deviceContext, IEffect* effect, const DirectX::FXMMATRIX& w, int InputDataType)
{
	std::shared_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);
	size_t sz = m_vecMeshdatas.size();
	for (size_t i = 0; i < sz; ++i)
	{
		{
			std::shared_lock<std::shared_mutex>LockerMaterial(m_mtxMaterial);
			IEffectMaterial* pEffectMaterial = dynamic_cast<IEffectMaterial*>(effect);
			if (pEffectMaterial && m_vecMaterial.size() > m_vecMeshdatas[i].m_MaterialIndex)
			{
				pEffectMaterial->SetMaterial(m_vecMaterial[m_vecMeshdatas[i].m_MaterialIndex]);
			}
		}

		IEffectTransform* pEffectTransform = dynamic_cast<IEffectTransform*>(effect);
		if (pEffectTransform)
		{
			pEffectTransform->SetWorldMatrix(w);
		}

		MeshDataInput input;
		IEffectMeshData* pEffectMeshData = dynamic_cast<IEffectMeshData*>(effect);
		if (pEffectMeshData)
		{
			input = pEffectMeshData->GetInputData(m_vecMeshdatas[i], InputDataType);
		}

		effect->Apply(deviceContext);

		deviceContext->IASetInputLayout(input.pInputLayout);
		deviceContext->IASetPrimitiveTopology(input.topology);
		deviceContext->IASetVertexBuffers(0, (uint32_t)input.pVertexBuffers.size(),
			input.pVertexBuffers.data(), input.strides.data(), input.offsets.data());
		deviceContext->IASetIndexBuffer(input.pIndexBuffer, input.indexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);

		deviceContext->DrawIndexed(input.indexCount, 0, 0);
	}
}

void CModel::CreateFromFile(CModel& model, ID3D11Device* device, std::string_view filename, const DirectX::XMFLOAT4& PickColor)
{
    using namespace Assimp;

	std::unique_lock<std::shared_mutex>LockerMeahData(model.m_mtxMeshdatas);
	std::unique_lock<std::shared_mutex>LockerMaterial(model.m_mtxMaterial);

    model.m_vecMaterial.clear();
    model.m_vecMeshdatas.clear();
    model.boundingbox = DirectX::BoundingBox();

	// 默认材质
	model.m_vecMaterial = { Material{} };
	model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$AmbientColor", DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.0f));
	model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$DiffuseColor", DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$SpecularColor", DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	model.m_vecMaterial[0].Set<float>("$SpecularFactor", 10.0f);
	model.m_vecMaterial[0].Set<float>("$Opacity", 1.0f);

	std::vector<DirectX::XMFLOAT4> pickColors;

    Importer importer;
    // 去掉里面的点、线图元
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

	auto pAssimpScene = importer.ReadFile(filename.data(),
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenBoundingBoxes |
		aiProcess_Triangulate |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices |
		aiProcess_GlobalScale);

    if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
    {
        model.m_vecMeshdatas.resize(pAssimpScene->mNumMeshes);
        model.m_vecMaterial.resize(pAssimpScene->mNumMaterials);
        for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
        {
            auto& mesh = model.m_vecMeshdatas[i];

            auto pAiMesh = pAssimpScene->mMeshes[i];
            uint32_t numVertices = pAiMesh->mNumVertices;

            CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
            D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };

			// 拾取颜色
			pickColors.clear();
			pickColors.resize(numVertices, PickColor);
			initData.pSysMem = pickColors.data();
			bufferDesc.ByteWidth = (uint32_t)pickColors.size() * sizeof(DirectX::XMFLOAT4);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pPickColors.GetAddressOf());

            // 位置
            if (pAiMesh->mNumVertices > 0)
            {
                initData.pSysMem = pAiMesh->mVertices;
                bufferDesc.ByteWidth = numVertices * sizeof(DirectX::XMFLOAT3);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pVertices.GetAddressOf());
				mesh.m_VertexCount = numVertices;

				DirectX::BoundingBox::CreateFromPoints(mesh.m_BoundingBox, numVertices,
                    (const DirectX::XMFLOAT3*)pAiMesh->mVertices, sizeof(DirectX::XMFLOAT3));
                if (i == 0)
                    model.boundingbox = mesh.m_BoundingBox;
                else
                    model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh.m_BoundingBox);
            }

            // 法线
            if (pAiMesh->HasNormals())
            {
                initData.pSysMem = pAiMesh->mNormals;
                bufferDesc.ByteWidth = numVertices * sizeof(DirectX::XMFLOAT3);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pNormals.GetAddressOf());
            }

            // 切线和副切线
            if (pAiMesh->HasTangentsAndBitangents())
            {
                std::vector<DirectX::XMFLOAT4> tangents(numVertices, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(DirectX::XMFLOAT3),
                        pAiMesh->mTangents + i, sizeof(DirectX::XMFLOAT3));
                }

                initData.pSysMem = tangents.data();
                bufferDesc.ByteWidth = pAiMesh->mNumVertices * sizeof(DirectX::XMFLOAT4);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTangents.GetAddressOf());

                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(DirectX::XMFLOAT3),
                        pAiMesh->mBitangents + i, sizeof(DirectX::XMFLOAT3));
                }
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pBitangents.GetAddressOf());
            }

            // 纹理坐标
            uint32_t numUVs = 8;
            while (numUVs && !pAiMesh->HasTextureCoords(numUVs - 1))
                numUVs--;

            if (numUVs > 0)
            {
                mesh.m_pTexcoordArrays.resize(numUVs);
                for (uint32_t i = 0; i < numUVs; ++i)
                {
                    std::vector<DirectX::XMFLOAT2> uvs(numVertices);
                    for (uint32_t j = 0; j < numVertices; ++j)
                    {
                        memcpy_s(&uvs[j], sizeof(DirectX::XMFLOAT2),
                            pAiMesh->mTextureCoords[i] + j, sizeof(DirectX::XMFLOAT2));
                    }
                    initData.pSysMem = uvs.data();
                    bufferDesc.ByteWidth = numVertices * sizeof(DirectX::XMFLOAT2);
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTexcoordArrays[i].GetAddressOf());
                }
            }

            // 索引
            uint32_t numFaces = pAiMesh->mNumFaces;
            uint32_t numIndices = numFaces * 3;
            if (numFaces > 0)
            {
                mesh.m_IndexCount = numIndices;
                if (numIndices < 65535)
                {
                    std::vector<uint16_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        indices[i * 3] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[0]);
                        indices[i * 3 + 1] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[1]);
                        indices[i * 3 + 2] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[2]);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                }
                else
                {
                    std::vector<uint32_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        memcpy_s(indices.data() + i * 3, sizeof(uint32_t) * 3,
                            pAiMesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                }
            }

            // 材质索引
            mesh.m_MaterialIndex = pAiMesh->mMaterialIndex;
        }


        for (uint32_t i = 0; i < pAssimpScene->mNumMaterials; ++i)
        {
            auto& material = model.m_vecMaterial[i];

            auto pAiMaterial = pAssimpScene->mMaterials[i];
			DirectX::XMFLOAT4 vec{};
            float value{};
            uint32_t boolean{};
            uint32_t num = 3;

            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, (float*)&vec, &num))
                material.Set("$AmbientColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
                material.Set("$DiffuseColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
                material.Set("$SpecularColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_SPECULAR_FACTOR, value))
                material.Set("$SpecularFactor", value);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
                material.Set("$EmissiveColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_OPACITY, value))
                material.Set("$Opacity", value);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
                material.Set("$TransparentColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
                material.Set("$ReflectiveColor", vec);
            
            aiString aiPath;
			std::filesystem::path texFilename;
            std::string texName;

            auto TryCreateTexture = [&](aiTextureType type, std::string_view propertyName, bool genMips = false, bool forceSRGB = false) {
                if (!pAiMaterial->GetTextureCount(type))
                    return;

                pAiMaterial->GetTexture(type, 0, &aiPath);

                // 纹理已经预先加载进来
                if (aiPath.data[0] == '*')
                {
                    texName = filename;
                    texName += aiPath.C_Str();
                    char* pEndStr = nullptr;
                    aiTexture* pTex = pAssimpScene->mTextures[strtol(aiPath.data + 1, &pEndStr, 10)];
                    TextureManager::Get()->CreateFromMemory(texName, pTex->pcData, pTex->mHeight ? pTex->mWidth * pTex->mHeight : pTex->mWidth, genMips, forceSRGB);
                    material.Set(propertyName, std::string(texName));
                }
                // 纹理通过文件名索引
                else
                {
                    texFilename = filename;
                    texFilename = texFilename.parent_path() / aiPath.C_Str();
                    TextureManager::Get()->CreateFromFile(texFilename.string(), genMips, forceSRGB);
                    material.Set(propertyName, texFilename.string());
                }
            };

            TryCreateTexture(aiTextureType_DIFFUSE, "$Diffuse", true, true);
            TryCreateTexture(aiTextureType_NORMALS, "$Normal");
            TryCreateTexture(aiTextureType_BASE_COLOR, "$Albedo", true, true);
            TryCreateTexture(aiTextureType_NORMAL_CAMERA, "$NormalCamera");
            TryCreateTexture(aiTextureType_METALNESS, "$Metalness");
            TryCreateTexture(aiTextureType_DIFFUSE_ROUGHNESS, "$Roughness");
            TryCreateTexture(aiTextureType_AMBIENT_OCCLUSION, "$AmbientOcclusion");
        }
		
    }
    else
    {
        std::string warning = "[Warning]: ModelManager::CreateFromFile, failed to load \"";
        warning += filename;
        warning += "\"\n";

        /*if (ImGuiLog::HasInstance())
        {
            ImGuiLog::Get().AddLog(warning.c_str());
        }
        else*/
        {
            OutputDebugStringA(warning.c_str());
        }
    }
}

std::vector<DirectX::XMFLOAT3> CModel::CreateWithAnchorFromFile(CModel& model, ID3D11Device* device, const std::string& filename, const DirectX::XMFLOAT4& PickColor)
{
	using namespace Assimp;

	std::unique_lock<std::shared_mutex>LockerMeahData(model.m_mtxMeshdatas);
	std::unique_lock<std::shared_mutex>LockerMaterial(model.m_mtxMaterial);

	model.m_vecMaterial.clear();
	model.m_vecMeshdatas.clear();
	model.boundingbox = DirectX::BoundingBox();

	// 默认材质
	model.m_vecMaterial = { Material{} };
	model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$AmbientColor", DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.0f));
	model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$DiffuseColor", DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$SpecularColor", DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	model.m_vecMaterial[0].Set<float>("$SpecularFactor", 10.0f);
	model.m_vecMaterial[0].Set<float>("$Opacity", 1.0f);

	std::vector<DirectX::XMFLOAT4> pickColors;
	std::vector<DirectX::XMFLOAT3> vecAnchorCenters;
	vecAnchorCenters.resize(EPMAT_Count);
	bool bIsGltfOrGlb = false;
	{
		// 仅按字节判断扩展名，规避 Windows 本地代码页转换异常
		std::string fileExt;
		const size_t slashPos = filename.find_last_of("/\\");
		const size_t dotPos = filename.find_last_of('.');
		if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos))
		{
			fileExt = filename.substr(dotPos);
		}
		std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		bIsGltfOrGlb = (fileExt == ".gltf" || fileExt == ".glb");
	}

	Importer importer;
	// 去掉里面的点、线图元
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

	auto pAssimpScene = importer.ReadFile(filename.data(),
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenBoundingBoxes |
		aiProcess_Triangulate |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices |
		aiProcess_GlobalScale|
		aiProcess_GenNormals);

	if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
	{
		DirectX::XMFLOAT3 center(0, 0, 0);
		int numPoint = 0;

		model.m_vecMeshdatas.resize(pAssimpScene->mNumMeshes);
		model.m_vecMaterial.resize(pAssimpScene->mNumMaterials);
		for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
		{
			auto& mesh = model.m_vecMeshdatas[i];

			auto pAiMesh = pAssimpScene->mMeshes[i];
			uint32_t numVertices = pAiMesh->mNumVertices;

			CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
			D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };

			// 拾取颜色
			pickColors.clear();
			pickColors.resize(numVertices, PickColor);
			initData.pSysMem = pickColors.data();
			bufferDesc.ByteWidth = (uint32_t)pickColors.size() * sizeof(DirectX::XMFLOAT4);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pPickColors.GetAddressOf());

			uint32_t numFaces = pAiMesh->mNumFaces;
			uint32_t numIndices = numFaces * 3;

			// 位置
			if (pAiMesh->mNumVertices > 0)
			{
				initData.pSysMem = pAiMesh->mVertices;
				bufferDesc.ByteWidth = numVertices * sizeof(DirectX::XMFLOAT3);
				device->CreateBuffer(&bufferDesc, &initData, mesh.m_pVertices.GetAddressOf());

				DirectX::BoundingBox::CreateFromPoints(mesh.m_BoundingBox, numVertices,
					(const DirectX::XMFLOAT3*)pAiMesh->mVertices, sizeof(DirectX::XMFLOAT3));
				if (i == 0)
					model.boundingbox = mesh.m_BoundingBox;
				else
					model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh.m_BoundingBox);

				for (unsigned int j = 0; j < numVertices; ++j) {

					center = DirectX::XMFLOAT3(pAiMesh->mVertices[j].x + center.x, pAiMesh->mVertices[j].y + center.y, pAiMesh->mVertices[j].z + center.z);

					numPoint++;
				}
			}

			// 法线
			if (pAiMesh->HasNormals())
			{
				initData.pSysMem = pAiMesh->mNormals;
				bufferDesc.ByteWidth = numVertices * sizeof(DirectX::XMFLOAT3);
				device->CreateBuffer(&bufferDesc, &initData, mesh.m_pNormals.GetAddressOf());
			}

			// 切线和副切线
			if (pAiMesh->HasTangentsAndBitangents())
			{
				std::vector<DirectX::XMFLOAT4> tangents(numVertices, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
				{
					memcpy_s(&tangents[i], sizeof(DirectX::XMFLOAT3),
						pAiMesh->mTangents + i, sizeof(DirectX::XMFLOAT3));
				}

				initData.pSysMem = tangents.data();
				bufferDesc.ByteWidth = pAiMesh->mNumVertices * sizeof(DirectX::XMFLOAT4);
				device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTangents.GetAddressOf());

				for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
				{
					memcpy_s(&tangents[i], sizeof(DirectX::XMFLOAT3),
						pAiMesh->mBitangents + i, sizeof(DirectX::XMFLOAT3));
				}
				device->CreateBuffer(&bufferDesc, &initData, mesh.m_pBitangents.GetAddressOf());
			}

			// 纹理坐标
			uint32_t numUVs = 8;
			while (numUVs && !pAiMesh->HasTextureCoords(numUVs - 1))
				numUVs--;

			if (numUVs > 0)
			{
				mesh.m_pTexcoordArrays.resize(numUVs);
				for (uint32_t i = 0; i < numUVs; ++i)
				{
					std::vector<DirectX::XMFLOAT2> uvs(numVertices);
					for (uint32_t j = 0; j < numVertices; ++j)
					{
						memcpy_s(&uvs[j], sizeof(DirectX::XMFLOAT2),
							pAiMesh->mTextureCoords[i] + j, sizeof(DirectX::XMFLOAT2));
					}
					initData.pSysMem = uvs.data();
					bufferDesc.ByteWidth = numVertices * sizeof(DirectX::XMFLOAT2);
					device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTexcoordArrays[i].GetAddressOf());
				}
			}

			// 索引
			if (numFaces > 0)
			{
				mesh.m_IndexCount = numIndices;
				if (numIndices < 65535)
				{
					std::vector<uint16_t> indices(numIndices);
					for (size_t i = 0; i < numFaces; ++i)
					{
						indices[i * 3] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[0]);
						indices[i * 3 + 1] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[1]);
						indices[i * 3 + 2] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[2]);
					}
					bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
					initData.pSysMem = indices.data();
					device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
				}
				else
				{
					std::vector<uint32_t> indices(numIndices);
					for (size_t i = 0; i < numFaces; ++i)
					{
						memcpy_s(indices.data() + i * 3, sizeof(uint32_t) * 3,
							pAiMesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
					}
					bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
					initData.pSysMem = indices.data();
					device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
				}
			}

			// 材质索引
			mesh.m_MaterialIndex = pAiMesh->mMaterialIndex;
		}

		vecAnchorCenters[0] = model.boundingbox.Center;
		vecAnchorCenters[1] = DirectX::XMFLOAT3(model.boundingbox.Center.x, model.boundingbox.Center .y - model.boundingbox.Extents.y, model.boundingbox.Center.z);
		vecAnchorCenters[2] = DirectX::XMFLOAT3(center.x / numPoint, center.y / numPoint, center.z / numPoint);

		vecAnchorCenters[EPMAT_OriginalCenter] = {0,0,0};
		vecAnchorCenters[EPMAT_GeometryCenter] = DirectX::XMFLOAT3(center.x / numPoint, center.y / numPoint, center.z / numPoint);

		vecAnchorCenters[EPMAT_TopCenter] = DirectX::XMFLOAT3(model.boundingbox.Center.x, model.boundingbox.Center.y + model.boundingbox.Extents.y, model.boundingbox.Center.z);
		vecAnchorCenters[EPMAT_BottomCenter] = DirectX::XMFLOAT3(model.boundingbox.Center.x, model.boundingbox.Center.y - model.boundingbox.Extents.y, model.boundingbox.Center.z);
		vecAnchorCenters[EPMAT_LeftCenter] = DirectX::XMFLOAT3(model.boundingbox.Center.x + model.boundingbox.Extents.x, model.boundingbox.Center.y, model.boundingbox.Center.z);
		vecAnchorCenters[EPMAT_RightCenter] = DirectX::XMFLOAT3(model.boundingbox.Center.x - model.boundingbox.Extents.x, model.boundingbox.Center.y, model.boundingbox.Center.z);
		vecAnchorCenters[EPMAT_FrontCenter] = DirectX::XMFLOAT3(model.boundingbox.Center.x, model.boundingbox.Center.y, model.boundingbox.Center.z - model.boundingbox.Extents.z);
		vecAnchorCenters[EPMAT_BackCenter] = DirectX::XMFLOAT3(model.boundingbox.Center.x, model.boundingbox.Center.y, model.boundingbox.Center.z + model.boundingbox.Extents.z);



		for (uint32_t i = 0; i < pAssimpScene->mNumMaterials; ++i)
		{
			auto& material = model.m_vecMaterial[i];

			auto pAiMaterial = pAssimpScene->mMaterials[i];

			DirectX::XMFLOAT4 vec{};
			float value{};
			uint32_t boolean{};
			uint32_t num = 3;
			aiColor3D color3;

			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color3))
				material.Set("$DiffuseColor", DirectX::XMFLOAT4(color3.r, color3.g, color3.b, 1.0f));
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
				material.Set("$DiffuseColor", vec);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
				material.Set("$SpecularColor", vec);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_SPECULAR_FACTOR, value))
				material.Set("$SpecularFactor", value);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
				material.Set("$EmissiveColor", vec);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_OPACITY, value))
				material.Set("$Opacity", value);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_SHININESS, value))
				material.Set("$Shininess", value);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
				material.Set("$TransparentColor", vec);
			if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
				material.Set("$ReflectiveColor", vec);
			if (bIsGltfOrGlb)
			{
				// 1. 基础颜色（安全读取）
				aiColor4D baseColor(1, 1, 1, 1);
				if (pAiMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
					material.Set("$BaseColor", DirectX::XMFLOAT4(baseColor.r, baseColor.g, baseColor.b, baseColor.a));

				aiColor3D diffuse(1, 1, 1);
				if (pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
					material.Set("$DiffuseColor", DirectX::XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, 1.0f));

				aiColor3D emissive(0, 0, 0);
				if (pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
					material.Set("$EmissiveColor", DirectX::XMFLOAT4(emissive.r, emissive.g, emissive.b, 1.0f));

				// 2. PBR 参数
				float metallic = 1.0f, roughness = 1.0f;
				if (pAiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
					material.Set("$Metalness", metallic);

				if (pAiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
					material.Set("$Roughness", roughness);

				// 3. Alpha 模式（用 int 而非 string）
				aiString alphaMode;
				int alphaModeInt = 0; // 0 OPAQUE,1 MASK,2 BLEND
				float alphaCutoff = 0.5f;
				if (pAiMaterial->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
					std::string am = alphaMode.C_Str();
					if (am == "MASK") alphaModeInt = 1;
					else if (am == "BLEND") alphaModeInt = 2;
				}
				material.Set("$AlphaMode", alphaModeInt);
				pAiMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
				material.Set("$AlphaCutoff", alphaCutoff);

				// 4. 双面
				int doubleSided = 0;
				pAiMaterial->Get(AI_MATKEY_TWOSIDED, doubleSided);
				material.Set("$DoubleSided", doubleSided);

				// 5. 模型路径
				std::string modelPathStr(filename);
				std::string modelParentPath;
				size_t slashPos = modelPathStr.find_last_of("/\\");
				if (slashPos != std::string::npos)
					modelParentPath = modelPathStr.substr(0, slashPos + 1);

				// 6. 纹理加载函数
				// type: Assimp 纹理类型（如 BASE_COLOR/NORMALS 等）
				// propName: 主材质属性名（纹理加载成功后写入此 key）
				// sRGB: 是否按 sRGB 格式创建纹理资源
				// genMips: 是否生成 mipmap
				// aliasPropName: 可选别名属性名（与 propName 指向同一纹理）
				auto LoadTexture = [&](aiTextureType type, const char* propName, bool sRGB = false, bool genMips = true, const char* aliasPropName = nullptr) -> bool {
					aiString aiPath;
					if (pAiMaterial->GetTexture(type, 0, &aiPath) != AI_SUCCESS || aiPath.length == 0)
						return false;

					std::string texPath = aiPath.C_Str();

					if (texPath[0] == '*') {
						int index = std::atoi(texPath.c_str() + 1);
						if (index < 0 || index >= (int)pAssimpScene->mNumTextures) return false;
						aiTexture* pTex = pAssimpScene->mTextures[index];
						if (!pTex || !pTex->pcData) return false;

						std::string ext = std::string(".") + pTex->achFormatHint;
						std::string uniqueName = "embedded_" + std::to_string(i) + "_" + std::to_string(index) + ext;

						if (pTex->mHeight == 0)
						{
							// 压缩纹理：mWidth 为字节数，可直接走内存解码路径。
							const size_t dataSize = static_cast<size_t>(pTex->mWidth);
							TextureManager::Get()->CreateFromMemory(uniqueName, pTex->pcData, dataSize, genMips, sRGB);
						}
						else
						{
							// 原始像素纹理：不能走 stbi 内存解码，需要直接上传 RGBA 数据。
							const UINT texWidth = static_cast<UINT>(pTex->mWidth);
							const UINT texHeight = static_cast<UINT>(pTex->mHeight);
							if (texWidth == 0 || texHeight == 0)
								return false;

							CD3D11_TEXTURE2D_DESC texDesc(
								sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
								texWidth, texHeight, 1,
								genMips ? 0 : 1,
								D3D11_BIND_SHADER_RESOURCE | (genMips ? D3D11_BIND_RENDER_TARGET : 0),
								D3D11_USAGE_DEFAULT, 0, 1, 0,
								genMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

							Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
							if (FAILED(device->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf())))
								return false;

							Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
							device->GetImmediateContext(deviceContext.GetAddressOf());
							if (!deviceContext)
								return false;
							deviceContext->UpdateSubresource(tex.Get(), 0, nullptr, pTex->pcData, texWidth * sizeof(aiTexel), 0);

							CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
								D3D11_SRV_DIMENSION_TEXTURE2D,
								sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
							Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
							if (FAILED(device->CreateShaderResourceView(tex.Get(), &srvDesc, srv.GetAddressOf())))
								return false;

							if (genMips)
								deviceContext->GenerateMips(srv.Get());

							TextureManager::Get()->AddTexture(uniqueName, srv.Get());
						}

						material.Set(propName, uniqueName);
						if (aliasPropName)
							material.Set(aliasPropName, uniqueName);
						return true;
					}

					std::string fullPath = modelParentPath + texPath;
					std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
					TextureManager::Get()->CreateFromFile(fullPath, genMips, sRGB);
					material.Set(propName, fullPath);
					if (aliasPropName)
						material.Set(aliasPropName, fullPath);
					return true;
				};

				// 7. 标准 glTF 纹理加载流程
				// 基础色（标准 → 兼容）
				if (!LoadTexture(aiTextureType_BASE_COLOR, "$Albedo", true, true, "$Diffuse"))
					LoadTexture(aiTextureType_DIFFUSE, "$Albedo", true, true, "$Diffuse");

				// 法线（标准 → 兼容）
				if (!LoadTexture(aiTextureType_NORMALS, "$Normal"))
					LoadTexture(aiTextureType_NORMAL_CAMERA, "$Normal");

				// 金属度（标准）
				LoadTexture(aiTextureType_METALNESS, "$Metalness");

				// 粗糙度（标准）
				LoadTexture(aiTextureType_DIFFUSE_ROUGHNESS, "$Roughness");

				// AO（标准）
				LoadTexture(aiTextureType_AMBIENT_OCCLUSION, "$AmbientOcclusion");

				// 自发光（标准 → 兼容）
				if (!LoadTexture(aiTextureType_EMISSIVE, "$Emissive", true))
					LoadTexture(aiTextureType_LIGHTMAP, "$Emissive", true);

				// 8. 支持 glTF 标准 ORM 贴图（OcclusionRoughnessMetallic）
				if (!material.Has<std::string>("$Metalness") || !material.Has<std::string>("$Roughness")) {
					aiString combPath;
					if (pAiMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &combPath) == AI_SUCCESS) {
						std::string p = combPath.C_Str();
						if (p.find("metallicRoughness") != std::string::npos || p.find("occlusionRoughnessMetallic") != std::string::npos) {
							LoadTexture(aiTextureType_UNKNOWN, "$MetalRoughness");
						}
					}
				}

			}
			else
			{
				auto TryCreateTexture = [&](aiTextureType type, std::string_view propertyName, bool genMips = false, bool forceSRGB = false) {
					for (unsigned int i = 0; i < pAiMaterial->GetTextureCount(type); ++i)
					{
						aiString path;
						if (pAiMaterial->GetTexture(type, i, &path) == AI_SUCCESS) {

							std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
							QString str = QString::fromStdWString(converter.from_bytes(path.C_Str()));

							str.replace("\\", "/");
							if (str.startsWith("../"))
							{
								str = str.mid(2);
							}

							QFileInfo file(QString::fromStdString(filename));
							str = file.absolutePath() + "/" + str;

							std::string s(str.toLocal8Bit().data());
							std::cout << "Texture path: " << s << std::endl;

							TextureManager::Get()->CreateFromFile(s, genMips, forceSRGB);
							material.Set(propertyName, s);
						}
					}
				};

				TryCreateTexture(aiTextureType_DIFFUSE, "$Diffuse");
			}
		}

	}
	else
	{
		std::string warning = "[Warning]: ModelManager::CreateFromFile, failed to load \"";
		warning += filename;
		warning += "\"\n";

		/*if (ImGuiLog::HasInstance())
		{
			ImGuiLog::Get().AddLog(warning.c_str());
		}
		else*/
		{
			OutputDebugStringA(warning.c_str());
		}
	}

	return vecAnchorCenters;
}

void CModel::CreateFromGeometry(CModel& model, ID3D11Device* device, const SGeometryRenderData& data, bool isDynamic)
{
	std::unique_lock<std::shared_mutex>LockerMeahData(model.m_mtxMeshdatas);
	std::unique_lock<std::shared_mutex>LockerMaterial(model.m_mtxMaterial);

    // 默认材质
    model.m_vecMaterial = { Material{} };
    model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$AmbientColor", DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$DiffuseColor", DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
    model.m_vecMaterial[0].Set<DirectX::XMFLOAT4>("$SpecularColor", DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    model.m_vecMaterial[0].Set<float>("$SpecularFactor", 10.0f);
    model.m_vecMaterial[0].Set<float>("$Opacity", 1.0f);

    model.m_vecMeshdatas = { MeshData{} };
    model.m_vecMeshdatas[0].m_pTexcoordArrays.resize(data.texcoordArrays.size());
    model.m_vecMeshdatas[0].m_VertexCount = (uint32_t)data.vertices.size();
    model.m_vecMeshdatas[0].m_IndexCount = (uint32_t)(!data.indices16.empty() ? data.indices16.size() : data.indices32.size());
    model.m_vecMeshdatas[0].m_MaterialIndex = 0;

    CD3D11_BUFFER_DESC bufferDesc(0,
        D3D11_BIND_VERTEX_BUFFER,
        isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
        isDynamic ? D3D11_CPU_ACCESS_WRITE : 0);
    D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };

    initData.pSysMem = data.vertices.data();
    bufferDesc.ByteWidth = (uint32_t)data.vertices.size() * sizeof(DirectX::XMFLOAT3);
    device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[0].m_pVertices.GetAddressOf());

    if (!data.normals.empty())
    {
        initData.pSysMem = data.normals.data();
        bufferDesc.ByteWidth = (uint32_t)data.normals.size() * sizeof(DirectX::XMFLOAT3);
        device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[0].m_pNormals.GetAddressOf());
    }

	if (!data.texcoordArrays.empty())
	{
		model.m_vecMeshdatas[0].m_pTexcoordArrays.resize(data.texcoordArrays.size());
		for (size_t j = 0; j < data.texcoordArrays.size(); j++)
		{
			if (!data.texcoordArrays[j].empty())
			{
				initData.pSysMem = data.texcoordArrays[j].data();
				bufferDesc.ByteWidth = (uint32_t)data.texcoordArrays[j].size() * sizeof(DirectX::XMFLOAT2);

				device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[0].m_pTexcoordArrays[j].GetAddressOf());
			}
		}
	}

    if (!data.tangents.empty())
    {
        initData.pSysMem = data.tangents.data();
        bufferDesc.ByteWidth = (uint32_t)data.tangents.size() * sizeof(DirectX::XMFLOAT4);
        device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[0].m_pTangents.GetAddressOf());
    }

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0;
    if (!data.indices16.empty())
    {
        initData.pSysMem = data.indices16.data();
        bufferDesc = CD3D11_BUFFER_DESC((uint16_t)data.indices16.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
        device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[0].m_pIndices.GetAddressOf());
    }
    else
    {
        initData.pSysMem = data.indices32.data();
        bufferDesc = CD3D11_BUFFER_DESC((uint32_t)data.indices32.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
        device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[0].m_pIndices.GetAddressOf());
    }
}

void  CModel::CreateFromData(CModel& model, ID3D11Device* device, const std::vector<SGeometryRenderData>& GeometryDatas,
	const std::vector<Material>& Materials, const DirectX::XMFLOAT4& PickColor)
{
	std::unique_lock<std::shared_mutex>LockerMeahData(model.m_mtxMeshdatas);
	std::unique_lock<std::shared_mutex>LockerMaterial(model.m_mtxMaterial);

	HRESULT hr = S_OK;

	model.m_vecMaterial = Materials;

	model.m_vecMeshdatas.clear();
	model.boundingbox = DirectX::BoundingBox();

	model.m_vecMeshdatas.resize(GeometryDatas.size());

	std::vector<DirectX::XMFLOAT4> pickColors;

	for (size_t i = 0; i < GeometryDatas.size(); i++)
	{
		auto& data = GeometryDatas[i];

		CD3D11_BUFFER_DESC bufferDesc(0,
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE );
		D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };

		// 拾取颜色
		pickColors.clear();
		pickColors.resize(data.vertices.size(), PickColor);
		initData.pSysMem = pickColors.data();
		bufferDesc.ByteWidth = (uint32_t)pickColors.size() * sizeof(DirectX::XMFLOAT4);
		device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pPickColors.GetAddressOf());

		//位置
		initData.pSysMem = data.vertices.data();
		bufferDesc.ByteWidth = (uint32_t)data.vertices.size() * sizeof(DirectX::XMFLOAT3);
		device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pVertices.GetAddressOf());
		model.m_vecMeshdatas[i].m_VertexCount = (uint32_t)data.vertices.size();

		//法线
		if (!data.normals.empty())
		{
			initData.pSysMem = data.normals.data();
			bufferDesc.ByteWidth = (uint32_t)data.normals.size() * sizeof(DirectX::XMFLOAT3);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pNormals.GetAddressOf());
		}

		//纹理
		if (!data.texcoordArrays.empty())
		{
			model.m_vecMeshdatas[i].m_pTexcoordArrays.resize(data.texcoordArrays.size());
			for (size_t j = 0; j < data.texcoordArrays.size(); j++)
			{
				if (!data.texcoordArrays[j].empty())
				{
					initData.pSysMem = data.texcoordArrays[j].data();
					bufferDesc.ByteWidth = (uint32_t)data.texcoordArrays[j].size() * sizeof(DirectX::XMFLOAT2);

					device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pTexcoordArrays[j].GetAddressOf());
				}
			}
		}

		//切线
		if (!data.tangents.empty())
		{
			initData.pSysMem = data.tangents.data();
			bufferDesc.ByteWidth = (uint32_t)data.tangents.size() * sizeof(DirectX::XMFLOAT4);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pTangents.GetAddressOf());
		}

		//颜色
		if (!data.colors.empty())
		{
			initData.pSysMem = data.colors.data();
			bufferDesc.ByteWidth = (uint32_t)data.colors.size() * sizeof(DirectX::XMFLOAT4);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pColors.GetAddressOf());
		}

		//索引
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
		if (!data.indices16.empty())
		{
			initData.pSysMem = data.indices16.data();
			bufferDesc = CD3D11_BUFFER_DESC((uint16_t)data.indices16.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pIndices.GetAddressOf());
			model.m_vecMeshdatas[i].m_IndexCount = (uint32_t)data.indices16.size();
		}
		else
		{
			initData.pSysMem = data.indices32.data();
			bufferDesc = CD3D11_BUFFER_DESC((uint32_t)data.indices32.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
			device->CreateBuffer(&bufferDesc, &initData, model.m_vecMeshdatas[i].m_pIndices.GetAddressOf());
			model.m_vecMeshdatas[i].m_IndexCount = (uint32_t)data.indices32.size();
		}
	}
}

bool CModel::CopyFromModel(CModel& dstModel, CModel& srcModel, ID3D11Device* device, const DirectX::XMFLOAT4& PickColor)
{
	if (!device)
		return false;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	device->GetImmediateContext(deviceContext.ReleaseAndGetAddressOf());
	if (!deviceContext)
		return false;

	auto CloneBuffer = [&](ID3D11Buffer* srcBuffer)->Microsoft::WRL::ComPtr<ID3D11Buffer> {
		Microsoft::WRL::ComPtr<ID3D11Buffer> dstBuffer;
		if (!srcBuffer)
			return dstBuffer;

		D3D11_BUFFER_DESC desc{};
		srcBuffer->GetDesc(&desc);
		HRESULT hr = device->CreateBuffer(&desc, nullptr, dstBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			dstBuffer.Reset();
			return dstBuffer;
		}
		deviceContext->CopyResource(dstBuffer.Get(), srcBuffer);
		return dstBuffer;
	};

	std::vector<Material> materials;
	std::vector<MeshData> srcMeshes;
	std::vector<MeshData> meshes;
	DirectX::BoundingBox boundingbox;

	{
		std::shared_lock<std::shared_mutex> srcMaterialLock(srcModel.m_mtxMaterial);
		materials = srcModel.m_vecMaterial;
	}
	{
		std::shared_lock<std::shared_mutex> srcMeshLock(srcModel.m_mtxMeshdatas);
		srcMeshes = srcModel.m_vecMeshdatas;
		boundingbox = srcModel.boundingbox;
	}

	meshes.resize(srcMeshes.size());
	for (size_t i = 0; i < srcMeshes.size(); ++i)
	{
		const auto& srcMesh = srcMeshes[i];
		auto& mesh = meshes[i];

		mesh.m_pVertices = CloneBuffer(srcMesh.m_pVertices.Get());
		mesh.m_pNormals = CloneBuffer(srcMesh.m_pNormals.Get());
		mesh.m_pTangents = CloneBuffer(srcMesh.m_pTangents.Get());
		mesh.m_pBitangents = CloneBuffer(srcMesh.m_pBitangents.Get());
		mesh.m_pColors = CloneBuffer(srcMesh.m_pColors.Get());
		mesh.m_pIndices = CloneBuffer(srcMesh.m_pIndices.Get());

		mesh.m_pTexcoordArrays.resize(srcMesh.m_pTexcoordArrays.size());
		for (size_t uvIdx = 0; uvIdx < srcMesh.m_pTexcoordArrays.size(); ++uvIdx)
		{
			mesh.m_pTexcoordArrays[uvIdx] = CloneBuffer(srcMesh.m_pTexcoordArrays[uvIdx].Get());
		}

		mesh.m_VertexCount = srcMesh.m_VertexCount;
		if (mesh.m_VertexCount == 0 && mesh.m_pVertices)
		{
			D3D11_BUFFER_DESC vbDesc{};
			mesh.m_pVertices->GetDesc(&vbDesc);
			mesh.m_VertexCount = vbDesc.ByteWidth / sizeof(DirectX::XMFLOAT3);
		}
		mesh.m_IndexCount = srcMesh.m_IndexCount;
		mesh.m_MaterialIndex = srcMesh.m_MaterialIndex;
		mesh.m_BoundingBox = srcMesh.m_BoundingBox;
		mesh.m_InFrustum = srcMesh.m_InFrustum;

		if (mesh.m_VertexCount > 0)
		{
			std::vector<DirectX::XMFLOAT4> pickColors(mesh.m_VertexCount, PickColor);
			CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
			D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
			initData.pSysMem = pickColors.data();
			bufferDesc.ByteWidth = static_cast<UINT>(pickColors.size() * sizeof(DirectX::XMFLOAT4));
			HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, mesh.m_pPickColors.ReleaseAndGetAddressOf());
			if (FAILED(hr))
				return false;
		}
		else
		{
			mesh.m_pPickColors.Reset();
		}
	}

	{
		std::unique_lock<std::shared_mutex> dstMaterialLock(dstModel.m_mtxMaterial);
		dstModel.m_vecMaterial = std::move(materials);
	}
	{
		std::unique_lock<std::shared_mutex> dstMeshLock(dstModel.m_mtxMeshdatas);
		dstModel.m_vecMeshdatas = std::move(meshes);
		dstModel.boundingbox = boundingbox;
	}

	return dstModel.GetMaterialSize() > 0 && dstModel.GetMeshDataSize() > 0;
}

void CModel::SetDebugObjectName(std::string_view name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	std::shared_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);
    std::string baseStr = name.data();
    size_t sz = m_vecMeshdatas.size();
    std::string str;
    str.reserve(100);
    for (size_t i = 0; i < sz; ++i)
    {
        baseStr = name.data();
        baseStr += "[" + std::to_string(i) + "].";
        if (m_vecMeshdatas[i].m_pVertices)
            D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pVertices.Get(), baseStr + "vertices");
        if (m_vecMeshdatas[i].m_pNormals)
			D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pNormals.Get(), baseStr + "normals");
        if (m_vecMeshdatas[i].m_pTangents)
			D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pTangents.Get(), baseStr + "tangents");
        if (m_vecMeshdatas[i].m_pBitangents)
			D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pBitangents.Get(), baseStr + "bitangents");
        if (m_vecMeshdatas[i].m_pColors)
			D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pColors.Get(), baseStr + "colors");
        if (!m_vecMeshdatas[i].m_pTexcoordArrays.empty())
        {
            size_t texSz = m_vecMeshdatas[i].m_pTexcoordArrays.size();
            for (size_t j = 0; j < texSz; ++j)
				D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pTexcoordArrays[j].Get(), baseStr + "uv" + std::to_string(j));
        }
        if (m_vecMeshdatas[i].m_pIndices)
			D3D11SetDebugObjectName(m_vecMeshdatas[i].m_pIndices.Get(), baseStr + "indices");
    }
#else
    UNREFERENCED_PARAMETER(name);
#endif
}

size_t CModel::GetMeshDataSize()
{
	std::shared_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);
	return m_vecMeshdatas.size();
}

size_t CModel::GetMaterialSize()
{
	std::shared_lock<std::shared_mutex>LockerMaterial(m_mtxMaterial);
	return m_vecMaterial.size();
}

bool CModel::GetMaterialByIndex(int nIndex, Material& material)
{
	std::shared_lock<std::shared_mutex>LockerMaterial(m_mtxMaterial);
	if(nIndex < 0 || nIndex >= m_vecMaterial.size())
		return false;
	material = m_vecMaterial[nIndex];
	return true;
}

bool CModel::GetMeshDataByIndex(int nIndex, MeshData& meshData)
{
	std::shared_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);
	if (nIndex < 0 || nIndex >= m_vecMeshdatas.size())
		return false;
	meshData = m_vecMeshdatas[nIndex];
	return true;
}

void CModel::UpdateTexSamplePos(ID3D11DeviceContext* pDeviceContext, const SGeometryRenderData& geometryDataCpu)
{
	if (!pDeviceContext)
	{
		return;
	}

	std::unique_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);
	if (m_vecMeshdatas.empty())
	{
		return;
	}

	auto& geometryDataGpu = m_vecMeshdatas[0];
	if (geometryDataGpu.m_pTexcoordArrays.empty() || !geometryDataGpu.m_pTexcoordArrays[0] || geometryDataCpu.texcoordArrays.empty())
	{
		return;
	}

	const size_t srcByteSize = sizeof(DirectX::XMFLOAT2) * geometryDataCpu.texcoordArrays[0].size();
	if (srcByteSize == 0)
	{
		return;
	}

	D3D11_BUFFER_DESC texcoordBufferDesc{};
	geometryDataGpu.m_pTexcoordArrays[0]->GetDesc(&texcoordBufferDesc);
	const size_t dstByteSize = static_cast<size_t>(texcoordBufferDesc.ByteWidth);
	assert(dstByteSize == srcByteSize);
	if (dstByteSize != srcByteSize)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedData{};
	const HRESULT hr = pDeviceContext->Map(geometryDataGpu.m_pTexcoordArrays[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	if (FAILED(hr) || !mappedData.pData)
	{
		return;
	}

	memcpy_s(mappedData.pData, dstByteSize, geometryDataCpu.texcoordArrays[0].data(), srcByteSize);
	pDeviceContext->Unmap(geometryDataGpu.m_pTexcoordArrays[0].Get(), 0);
}

void CModel::UpdateTexColor(ID3D11DeviceContext* pDeviceContext, const SGeometryRenderData& geometryDataCpu)
{
	if (!pDeviceContext)
	{
		return;
	}

	std::unique_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);
	if (m_vecMeshdatas.empty())
	{
		return;
	}

	auto& geometryDataGpu = m_vecMeshdatas[0];
	if (!geometryDataGpu.m_pColors || geometryDataCpu.colors.empty())
	{
		return;
	}

	const size_t srcByteSize = sizeof(DirectX::XMFLOAT4) * geometryDataCpu.colors.size();
	if (srcByteSize == 0)
	{
		return;
	}

	D3D11_BUFFER_DESC colorBufferDesc{};
	geometryDataGpu.m_pColors->GetDesc(&colorBufferDesc);
	const size_t dstByteSize = static_cast<size_t>(colorBufferDesc.ByteWidth);
	//assert(dstByteSize == srcByteSize);
	if (dstByteSize != srcByteSize)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedData{};
	const HRESULT hr = pDeviceContext->Map(geometryDataGpu.m_pColors.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	if (FAILED(hr) || !mappedData.pData)
	{
		return;
	}

	memcpy_s(mappedData.pData, dstByteSize, geometryDataCpu.colors.data(), srcByteSize);
	pDeviceContext->Unmap(geometryDataGpu.m_pColors.Get(), 0);
}

void CModel::SetMaterial(int nIndex, const Material& meterial)
{
	std::unique_lock<std::shared_mutex>LockerMaterial(m_mtxMaterial);

	if (nIndex < 0 || nIndex >= m_vecMaterial.size())
		return;
	m_vecMaterial[nIndex] = meterial;
}

void CModel::SetMeahData(int nIndex, const MeshData& meshData)
{
	std::unique_lock<std::shared_mutex>LockerMeahData(m_mtxMeshdatas);

	if (nIndex < 0 || nIndex >= m_vecMeshdatas.size())
		return;
	m_vecMeshdatas[nIndex] = meshData;
}