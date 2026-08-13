#pragma once

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include "CModel.h"

#include <QUuid>

#include <d3d11_1.h>
#include <wrl/client.h>
#include <map>
#include <set>
#include <shared_mutex>

class RENDERCOMMON_API ModelManager
{
private:
	ModelManager();

public:
	~ModelManager();
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	ModelManager(ModelManager&&) = default;
	ModelManager& operator=(ModelManager&&) = default;

	static ModelManager* GetInstance();
	void Init(ID3D11Device* device);
	void UnInit();


	void DrawModel(const QUuid& id, ID3D11DeviceContext* deviceContext, IEffect* effect, const DirectX::FXMMATRIX& w, int InputDataType);

	///模型GPU数据创建函数
	// 返回创建模型数据是否为空
	bool CreateFromFile(const QUuid& id, std::string_view filename, const DirectX::XMFLOAT4& PickColor);

	bool CreateFromData(const QUuid& id, const SGeometryRenderData& data,
		const Material& Materials, const DirectX::XMFLOAT4& PickColor,  const std::string_view& strDebugName);
	
	bool CreateWithAnchorFromFile(const QUuid& id, const std::string& filename, 
		const DirectX::XMFLOAT4& PickColor, std::vector<DirectX::XMFLOAT3>& vecAnchorCenters, const std::string_view &strDebugName);
	bool CopyFromModel(const QUuid& srcId, const QUuid& dstId, const DirectX::XMFLOAT4& PickColor, const std::string_view& strDebugName);

	///模型删除函数
	void ClearModel();
	void EraseModel(const QUuid& id);

	///属性设置、获取与更新
	void SetModelDefaultMaterial(const QUuid& id, const Material& Materials);
	void SetModelMaterial(const QUuid& id, int nIndx, std::string_view strKey, std::string strValue);
	void SetModelMaterial(const QUuid& id, int nIndx, std::string_view strKey,int nValue);
	void SetModelDebugName(const QUuid& id, const std::string_view& strDebugName);

	int GetModelMaterialCount(const QUuid& id);
	bool GetModelMaterial(const QUuid& id, int nIndex, Material& Materials);
	bool GetModelMeshData(const QUuid& id, int nIndex, MeshData& Materials);
	void GetAllModelsID(std::set<QUuid>& setIDs);

	void UpdateModelTexSamplePos(const QUuid& id, const SGeometryRenderData& data);
	void UpdateModelTexSampleColor(const QUuid& id, const SGeometryRenderData& data);

private:

	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	std::map<QUuid, CModel> m_mapModel;
	std::shared_mutex m_mtxModels;
};


#define g_pDrawModelMgr ModelManager::GetInstance()

#endif // MODELMANAGER_H
