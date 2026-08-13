
#ifndef MODEL_H
#define MODEL_H

#include "RenderCommonGlobal.h"
#include <vector>
#include <string>
#include <wrl/client.h>
#include <DirectXCollision.h>
#include "d3d11_1.h"

struct ModelPart
{
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ModelPart() :vertexBuffer(), indexBuffer(),
		vertexCount(), indexCount(), indexFormat(){}

	ModelPart(const ModelPart&) = default;
	ModelPart& operator=(const ModelPart&) = default;

	ModelPart(ModelPart&&) = default;
	ModelPart& operator=(ModelPart&&) = default;

	void Reset()
	{
		vertexBuffer.Reset();
		indexBuffer.Reset();
	}

	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	UINT vertexCount;
	UINT indexCount;

	DXGI_FORMAT indexFormat;
};

struct RENDERCOMMON_API Model
{
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	Model();

	void Reset();

	template<class VertexType, class IndexType>
	Model(ID3D11Device * device, const std::vector<VertexType> & vertices, const std::vector<IndexType>& indices);
	
	Model(ID3D11Device * device, const void* vertices, UINT vertexSize, UINT vertexCount,
		const void * indices, UINT indexCount, DXGI_FORMAT indexFormat);

	//
	// 设置网格
	//
	template<class VertexType, class IndexType>
	void AddMesh(ID3D11Device * device, const std::vector<VertexType> & vertices, const std::vector<IndexType>& indices);

	void AddMesh(ID3D11Device * device, const void* vertices, UINT vertexSize, UINT vertexCount,
		const void * indices, UINT indexCount, DXGI_FORMAT indexFormat);

	//更新顶点数据
	void UpdataVertexBuffer(ID3D11DeviceContext* deviceContex, UINT nIndex, const void* vertices);

	//
	// 调试 
	//

	// 设置调试对象名
	// 若模型被重新设置，调试对象名也需要被重新设置
	void SetDebugObjectName(const std::string& name);

	std::vector<ModelPart> modelParts;
	UINT vertexStride;
};


template<class VertexType, class IndexType>
inline Model::Model(ID3D11Device * device, const std::vector<VertexType> & vertices, const std::vector<IndexType>& indices)
	: modelParts()
{
	AddMesh(device, vertices, indices);
}

template<class VertexType, class IndexType>
inline void Model::AddMesh(ID3D11Device * device, const std::vector<VertexType> & vertices, const std::vector<IndexType>& indices)
{
	static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes!");
	static_assert(std::is_unsigned<IndexType>::value, "IndexType must be unsigned integer!");
	AddMesh(device, vertices.data(), sizeof(VertexType),
		(UINT)vertices.size(), indices.data(), (UINT)indices.size(),
		(sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT));
}

#endif
