#include "Model.h"
#include "GSMedia/RenderCommon/d3dUtil.h"
#include "GSMedia/RenderCommon/DXTrace.h"

using namespace DirectX;

Model::Model()
	: modelParts(), vertexStride()
{
}

void Model::Reset()
{
	for (auto iter : modelParts)
		iter.Reset();
}

Model::Model(ID3D11Device* device, const void* vertices, UINT vertexSize, UINT vertexCount,
	const void* indices, UINT indexCount, DXGI_FORMAT indexFormat)
	: modelParts(), vertexStride()
{
	AddMesh(device, vertices, vertexSize, vertexCount, indices, indexCount, indexFormat);
}

void Model::AddMesh(ID3D11Device* device, const void* vertices, UINT vertexSize, UINT vertexCount, const void* indices, UINT indexCount, DXGI_FORMAT indexFormat)
{
	if (modelParts.size() == 0)
	{
		vertexStride = vertexSize;
	}
	else
	{
		assert(vertexSize == vertexStride);
		if (vertexStride != vertexSize)
		{
			//throw std::runtime_error{ "Bad format in vertices" };
			return;
		}
	}

	ModelPart modelPart;
	modelPart.vertexCount = vertexCount;
	modelPart.indexCount = indexCount;
	modelPart.indexFormat = indexFormat;

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = vertexSize * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &InitData, modelPart.vertexBuffer.ReleaseAndGetAddressOf()));

	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.CPUAccessFlags = 0;

	if (indexFormat == DXGI_FORMAT_R16_UINT)
	{
		ibd.ByteWidth = indexCount * (UINT)sizeof(WORD);
	}
	else
	{
		ibd.ByteWidth = indexCount * (UINT)sizeof(DWORD);
	}
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	// 新建索引缓冲区
	InitData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &InitData, modelPart.indexBuffer.ReleaseAndGetAddressOf()));

	modelParts.push_back(modelPart);
}

void Model::UpdataVertexBuffer(ID3D11DeviceContext* deviceContex, UINT nIndex, const void* vertices)
{
	assert(nIndex < modelParts.size());

	if (nIndex < modelParts.size())
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(deviceContex->Map(modelParts[nIndex].vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, vertexStride * modelParts[nIndex].vertexCount, vertices, vertexStride * modelParts[nIndex].vertexCount);
		deviceContex->Unmap(modelParts[nIndex].vertexBuffer.Get(), 0);
	}
}

void Model::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)

	size_t modelPartSize = modelParts.size();
	for (size_t i = 0; i < modelPartSize; ++i)
	{
		D3D11SetDebugObjectName(modelParts[i].vertexBuffer.Get(), name + ".part[" + std::to_string(i) + "].VertexBuffer");
		D3D11SetDebugObjectName(modelParts[i].indexBuffer.Get(), name + ".part[" + std::to_string(i) + "].IndexBuffer");
	}

#else
	UNREFERENCED_PARAMETER(name);
#endif
}


