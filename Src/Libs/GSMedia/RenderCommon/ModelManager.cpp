#include "ModelManager.h"

ModelManager::ModelManager()
{
}

ModelManager::~ModelManager()
{
}

ModelManager* ModelManager::GetInstance()
{
	static ModelManager s_Instance;
	return &s_Instance;
}

void ModelManager::Init(ID3D11Device* device)
{
	m_pDevice = device;
	m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());
}

void ModelManager::UnInit()
{
	ClearModel();
	m_pDeviceContext.Reset();
	m_pDevice.Reset();
}

void ModelManager::DrawModel(const QUuid& id, ID3D11DeviceContext* deviceContext, IEffect* effect, const DirectX::FXMMATRIX& w, int InputDataType)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.Draw(deviceContext, effect, w, InputDataType);
}

void ModelManager::ClearModel()
{
	std::unique_lock<std::shared_mutex> lock(m_mtxModels);
	m_mapModel.clear();
}

void ModelManager::SetModelDefaultMaterial(const QUuid& id, const Material& Materials)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);

	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.SetMaterial(0, Materials);
}

void ModelManager::SetModelMaterial(const QUuid& id, int nIndx, std::string_view strKey, std::string strValue)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.SetMaterialValue(nIndx, strKey, strValue);
}

void ModelManager::SetModelMaterial(const QUuid& id, int nIndx, std::string_view strKey, int nValue)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.SetMaterialValue(nIndx, strKey, nValue);
}

int ModelManager::GetModelMaterialCount(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return 0;
	return iter->second.GetMaterialSize();
}

bool ModelManager::GetModelMaterial(const QUuid& id, int nIndex, Material& materials)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return false;
	return iter->second.GetMaterialByIndex(nIndex, materials);
}

bool ModelManager::GetModelMeshData(const QUuid& id, int nIndex, MeshData& meshData)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return false;
	return iter->second.GetMeshDataByIndex(nIndex, meshData);
}

void ModelManager::SetModelDebugName(const QUuid& id, const std::string_view& strDebugName)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);

	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.SetDebugObjectName(strDebugName);
}

void ModelManager::UpdateModelTexSamplePos(const QUuid& id, const SGeometryRenderData& vecDataCpu)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.UpdateTexSamplePos(m_pDeviceContext.Get(), vecDataCpu);
}

void ModelManager::UpdateModelTexSampleColor(const QUuid& id, const SGeometryRenderData& vecDataCpu)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	auto& iter = m_mapModel.find(id);
	if (iter == m_mapModel.end())
		return;
	iter->second.UpdateTexColor(m_pDeviceContext.Get(), vecDataCpu);
}

bool ModelManager::CreateFromFile(const QUuid& id, std::string_view filename, const DirectX::XMFLOAT4& PickColor)
{
	std::unique_lock<std::shared_mutex> lock(m_mtxModels);

	auto& model = m_mapModel[id];
	CModel::CreateFromFile(model, m_pDevice.Get(), filename, PickColor);

	return model.GetMaterialSize() > 0 && model.GetMeshDataSize() > 0;
}

bool ModelManager::CreateFromData(const QUuid& id, const SGeometryRenderData& data,
	const Material& Materials, const DirectX::XMFLOAT4& PickColor, const std::string_view& strDebugName)
{
	std::unique_lock<std::shared_mutex> lock(m_mtxModels);

	auto& model = m_mapModel[id];
	CModel::CreateFromData(model, m_pDevice.Get(), { data }, { Materials }, PickColor);
	model.boundingbox = data.boundingbox;

	model.SetDebugObjectName(strDebugName);

	return model.GetMaterialSize() > 0 && model.GetMeshDataSize() > 0;
}

bool ModelManager::CreateWithAnchorFromFile(const QUuid& id, const std::string& filename,
	const DirectX::XMFLOAT4& PickColor, std::vector<DirectX::XMFLOAT3>& vecAnchorCenters, const std::string_view& strDebugName)
{
	std::unique_lock<std::shared_mutex> lock(m_mtxModels);

	auto& model = m_mapModel[id];
	vecAnchorCenters = CModel::CreateWithAnchorFromFile(model, m_pDevice.Get(), filename, PickColor);

	model.SetDebugObjectName(strDebugName);

	return model.GetMaterialSize() > 0 && model.GetMeshDataSize() > 0;
}

bool ModelManager::CopyFromModel(const QUuid& srcId, const QUuid& dstId, const DirectX::XMFLOAT4& PickColor, const std::string_view& strDebugName)
{
	if (srcId == dstId)
		return false;

	std::unique_lock<std::shared_mutex> lock(m_mtxModels);

	auto srcIter = m_mapModel.find(srcId);
	if (srcIter == m_mapModel.end())
		return false;

	auto& dstModel = m_mapModel[dstId];
	bool bRet = CModel::CopyFromModel(dstModel, srcIter->second, m_pDevice.Get(), PickColor);
	if (!bRet)
	{
		m_mapModel.erase(dstId);
		return false;
	}

	dstModel.SetDebugObjectName(strDebugName);
	return true;
}

void ModelManager::EraseModel(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtxModels);
	m_mapModel.erase(id);
}

void ModelManager::GetAllModelsID(std::set<QUuid>& setIDs)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxModels);
	setIDs.clear();
	for (auto it = m_mapModel.begin(); it != m_mapModel.end(); it++)
		setIDs.insert(it->first);
}
