//***************************************************************************************
// GameObject.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 简易游戏对象
// Simple game object.
//***************************************************************************************

#pragma once

#ifndef CMODEL_H
#define CMODEL_H

#include "Geometry.h"
#include "Material.h"
#include "MeshData.h"
#include "RenderCommonGlobal.h"
#include "IEffect.h"

#include <d3d11_1.h>
#include <shared_mutex>

class RENDERCOMMON_API CModel
{
public:
	CModel() = default;
	~CModel() = default;
	CModel(CModel&) = delete;
	CModel& operator=(const CModel&) = delete;
	CModel(CModel&&) = default;
	CModel& operator=(CModel&&) = default;

	DirectX::BoundingBox boundingbox;
	static void CreateFromGeometry(CModel& model, ID3D11Device* device, const SGeometryRenderData& data, bool isDynamic = false);
	static void CreateFromFile(CModel& model, ID3D11Device* device, std::string_view filename, const DirectX::XMFLOAT4& PickColor);
	static std::vector<DirectX::XMFLOAT3> CreateWithAnchorFromFile(CModel& model, ID3D11Device* device, const std::string& filename, const DirectX::XMFLOAT4& PickColor);
	static void CreateFromData(CModel& model, ID3D11Device* device, const std::vector<SGeometryRenderData>& data, const std::vector<Material>& Materials, const DirectX::XMFLOAT4& PickColor);
	static bool CopyFromModel(CModel& dstModel, CModel& srcModel, ID3D11Device* device, const DirectX::XMFLOAT4& PickColor);

	void Draw(ID3D11DeviceContext* deviceContext, IEffect* effect, const DirectX::FXMMATRIX& w, int InputDataType);
	void SetDebugObjectName(std::string_view name);

	size_t GetMeshDataSize();
	size_t GetMaterialSize();
	bool GetMaterialByIndex(int nIndex, Material& material);
	bool GetMeshDataByIndex(int nIndex, MeshData& meshData);
	void UpdateTexSamplePos(ID3D11DeviceContext* pDeviceContext, const SGeometryRenderData& data);
	void UpdateTexColor(ID3D11DeviceContext* pDeviceContext, const SGeometryRenderData& data);

	template<class T>
	void SetMaterialValue(int nIndex, std::string_view name, const T& value);

	void SetMaterial(int nIndex, const Material& meterial);
	void SetMeahData(int nIndex, const MeshData& meshData);

private:
	std::shared_mutex m_mtxMaterial;
	std::vector<Material> m_vecMaterial;
	std::shared_mutex m_mtxMeshdatas;
	std::vector<MeshData> m_vecMeshdatas;
};

template<class T>
inline void CModel::SetMaterialValue(int nIndex, std::string_view name, const T& value)
{
	std::unique_lock<std::shared_mutex> LockerMaterial(m_mtxMaterial);

	if (nIndex < 0 || nIndex >= m_vecMaterial.size())
		return;
	m_vecMaterial[nIndex].Set(name, value);
}

#endif // CMODEL_H
