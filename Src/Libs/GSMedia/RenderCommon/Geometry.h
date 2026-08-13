#pragma once
#include "RenderCommonGlobal.h"
#include "Common/Common/Maths/Point.h"

#include <memory>
#include <map>
#include <array>
#include <vector>
#include <string>
#include <numeric>
#include <functional>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#include "GeometryDefine.h"
#include "Vertex.h"

namespace Geometry
{
	// 创建球体网格数据，levels和slices越大，精度越高。
	RENDERCOMMON_API SGeometryRenderData CreateSphere(float radius = 1.0f, uint32_t levels = 20, uint32_t slices = 20,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建球体网格数据，levels和slices越大，精度越高。
	RENDERCOMMON_API GeometryData CreateSphereEx(float radius = 1.0f, uint32_t levels = 20, uint32_t slices = 20,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), bool bInward = true,
		float latitudeStartDeg = 0.0f, float latitudeEndDeg = 180.0f, float longitudeRangeDeg = 360.0f);

	// 创建立方体网格数据
	RENDERCOMMON_API SGeometryRenderData CreateBox(float width = 2.0f, float height = 2.0f, float depth = 2.0f,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建立方体网格数据（法线向内）
	RENDERCOMMON_API SGeometryRenderData CreateBoxInward(float width = 2.0f, float height = 2.0f, float depth = 2.0f,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建圆柱体网格数据，slices越大，精度越高。
	RENDERCOMMON_API SGeometryRenderData CreateCylinder(float radius = 1.0f, float height = 2.0f, uint32_t slices = 20, uint32_t stacks = 10, float texU = 1.0f, float texV = 1.0f,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建圆柱体侧面网格数据，slices越大，精度越高。
	RENDERCOMMON_API SGeometryRenderData CreateCirque(float radius = 1.0f, float height = 2.0f, uint32_t slices = 20, uint32_t stacks = 10, float texU = 1.0f, float texV = 1.0f,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建圆锥体网格数据，slices越大，精度越高。
	RENDERCOMMON_API SGeometryRenderData CreateCone(float radius = 1.0f, float height = 2.0f, uint32_t slices = 20,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建一个平面
	RENDERCOMMON_API GeometryData CreatePlane(const DirectX::XMFLOAT2& planeSize, const DirectX::XMFLOAT2& maxTexCoord = { 1.0f, 1.0f });
	RENDERCOMMON_API GeometryData CreatePlane(float width = 10.0f, float depth = 10.0f, float texU = 1.0f, float texV = 1.0f,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 创建一个网格
	RENDERCOMMON_API GeometryData CreateGrid(const DirectX::XMFLOAT2& gridSize, const DirectX::XMUINT2& slices, const DirectX::XMFLOAT2& maxTexCoord,
		const std::function<float(float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
		const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc = [](float x, float z) { return DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f); },
		const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc = [](float x, float z) { return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); });

	//创建一个圆环
	RENDERCOMMON_API SGeometryRenderData CreateTorus(float innerRadius = 1.0f, float outerRadius = 1.1f,
		uint32_t radialSegments = 32, uint32_t tubularSegments = 4,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 凸多边形轮廓允许的最大 R 角半径（mm）。
	// 顶点约束：min(l1,l2)*tan(θ/2)；边约束：L/(cot(α/2)+cot(β/2))；取全局最小值。
	/*
	功能：计算凸多边形轮廓允许的最大 R 角半径
	参数：
	  vecPoint - 底面轮廓顶点序列，单位 mm
	返回值：允许的最大 R 角半径，单位 mm；轮廓无效时返回 0
	*/
	RENDERCOMMON_API float CalRCornerRadius(std::vector<CPointD>& vecPoint);

	// 不规则柱体构建参数
	struct SIrregularPrismBuildParams
	{
		std::vector<CPointD> vecPoint;                          // 底面轮廓顶点，单位 mm
		std::vector<bool> vecFacePresent;                       // 各边是否生成柱面
		int nHeight = 1000;                                     // 柱体高度，单位 mm
		bool bClose = false;                                    // 轮廓是否闭合
		bool bConvex = false;                                   // 轮廓是否凸
		bool bBottomFace = true;                                // 是否生成底面
		bool bUpperFace = true;                                 // 是否生成顶面
		bool bNormalInward = true;                              // 法向是否朝内
		size_t nBaseSubdivisionAmount = 16;                     // R 角圆弧细分段数
		bool bBottomfaceCorner = false;                         // 底面是否启用 R 角
		bool bUpperfaceCorner = false;                          // 顶面是否启用 R 角
		int cornerRadius = 500;                                 // R 角半径，单位 mm
		DirectX::XMFLOAT4 color{ 0.f, 0.f, 0.f, 1.f };          // 默认顶点颜色
	};

	/*
	功能：根据构建参数创建不规则柱体几何数据（含柱面、顶底面及 R 角）
	参数：
	  buildParams - 不规则柱体构建参数，主要字段说明：
	    vecPoint               - 底面轮廓顶点，单位 mm
	    vecFacePresent         - 各边是否生成柱面
	    nHeight                - 柱体高度，单位 mm
	    bClose / bConvex       - 轮廓是否闭合 / 是否凸
	    bBottomFace/bUpperFace - 是否生成底面 / 顶面
	    bNormalInward          - 法向是否朝内
	    nBaseSubdivisionAmount - R 角圆弧细分段数
	    bBottomfaceCorner      - 底面是否启用 R 角
	    bUpperfaceCorner       - 顶面是否启用 R 角
	    cornerRadius           - R 角半径，单位 mm
	    color                  - 默认顶点颜色
	返回值：柱体几何数据；轮廓无效或周长为零时返回空 GeometryData
	*/
	RENDERCOMMON_API GeometryData CreateIrregularPrism(SIrregularPrismBuildParams& buildParams);

	//创建一个旋转体
	/*
	vecPointVector：旋转体的旋转切面，单位为mm
	vecClosed：单个旋转切面是否是闭合状态
	nSteps：旋转步数
	nAngle：旋转角度
	bNormalInward：法向朝内/朝外
	nSubdivisionAmount: 面细分成nSubdivisionAmount个小矩形
	*/
	RENDERCOMMON_API GeometryData CreateRotationSolid(
		const std::vector<std::vector<CPointD>>& vecPointVector,
		const std::vector<bool>& vecClosed,
		const int nSteps,
		const int nAngle,
		const bool bNormalInward,
		const int nSubdivisionAmount = 24,
		const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	// 网格数据
	template<class VertexType = VertexPosTex, class IndexType = DWORD>
	struct MeshData
	{
		std::vector<VertexType> vertexVec;	// 顶点数组
		std::vector<IndexType> indexVec;	// 索引数组

		MeshData()
		{
			// 需检验索引类型合法性
			static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes!");
			static_assert(std::is_unsigned<IndexType>::value, "IndexType must be unsigned integer!");
		}
	};

	// 创建一个指定NDC屏幕区域的面(默认全屏)
	template<class VertexType = VertexPosTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> Create2DShow(const DirectX::XMFLOAT2& center, const DirectX::XMFLOAT2& scale, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	template<class VertexType = VertexPosTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> Create2DShow(float centerX = 0.0f, float centerY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建一个带纹理坐标的面
	template<class VertexType = VertexPosTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateFrame();

	// 创建一个线段
	template<class VertexType = VertexPosColor, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateLine();

	// 创建一个空心矩形
	template<class VertexType = VertexPosColor, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateRect();

}

namespace Geometry
{
	namespace Internal
	{
		//
		// 以下结构体和函数仅供内部实现使用
		//

		struct VertexData
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 tex;
		};

		// 根据目标顶点类型选择性将数据插入
		template<class VertexType>
		inline void InsertVertexElement(VertexType& vertexDst, const VertexData& vertexSrc)
		{
			static std::string semanticName;
			static const std::map<std::string, std::pair<size_t, size_t>> semanticSizeMap = {
				{"POSITION", std::pair<size_t, size_t>(0, 12)},
				{"COLOR", std::pair<size_t, size_t>(12, 28)},
				{"TEXCOORD", std::pair<size_t, size_t>(28, 36)}
			};

			for (size_t i = 0; i < ARRAYSIZE(VertexType::inputLayout); i++)
			{
				semanticName = VertexType::inputLayout[i].SemanticName;
				const auto& range = semanticSizeMap.at(semanticName);
				memcpy_s(reinterpret_cast<char*>(&vertexDst) + VertexType::inputLayout[i].AlignedByteOffset,
					range.second - range.first,
					reinterpret_cast<const char*>(&vertexSrc) + range.first,
					range.second - range.first);
			}
		}
	}

	//
	// 几何体方法的实现
	//

	template<class VertexType, class IndexType>
	inline MeshData<VertexType, IndexType> Create2DShow(const DirectX::XMFLOAT2& center, const DirectX::XMFLOAT2 & scale, const DirectX::XMFLOAT4 & color)
	{
		return Create2DShow<VertexType, IndexType>(center.x, center.y, scale.x, scale.y, color);
	}

	template<class VertexType, class IndexType>
	inline MeshData<VertexType, IndexType> Create2DShow(float centerX, float centerY, float scaleX, float scaleY, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(4);

		Internal::VertexData vertexData;
		UINT vIndex = 0;

		vertexData = { XMFLOAT3(centerX - scaleX, centerY - scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(centerX - scaleX, centerY + scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(centerX + scaleX, centerY + scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(centerX + scaleX, centerY - scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		meshData.indexVec = { 0, 1, 2, 2, 3, 0 };
		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateFrame()
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(4);

		Internal::VertexData vertexData;
		UINT vIndex = 0;

		vertexData = { XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		meshData.indexVec = { 1,0,2,3,2,0 };
		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateLine()
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(4);

		Internal::VertexData vertexData;
		UINT vIndex = 0;

		vertexData = { XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		meshData.indexVec = { 0,1,2,3 };
		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateRect()
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(4);

		Internal::VertexData vertexData;
		UINT vIndex = 0;

		vertexData = { XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		meshData.indexVec = { 0, 1, 2, 3, 0 };
		return meshData;
	}
}