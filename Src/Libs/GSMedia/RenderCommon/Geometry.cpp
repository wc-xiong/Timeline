#include "Geometry.h"
#include "SimpleMath.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/algorithms/intersection.hpp>

#include "GeometryDefine.h"
#include "Common/Common/CommonFunctionMath.h"

#include <QDebug>
#include <algorithm>
#include <cfloat>
#define _USE_MATH_DEFINES
#include <math.h>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace Geometry
{
	constexpr float PI = 3.1415926f;
	//
	// 几何体方法的实现
	//

	SGeometryRenderData CreateSphere(float radius, uint32_t levels, uint32_t slices, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		SGeometryRenderData geoData;

		uint32_t vertexCount = 2 + (levels - 1) * (slices + 1);
		uint32_t indexCount = 6 * (levels - 1) * slices;
		geoData.vertices.resize(vertexCount);
		geoData.normals.resize(vertexCount);
		geoData.texcoordArrays.resize(1);
		geoData.texcoordArrays[0].resize(vertexCount);
		geoData.tangents.resize(vertexCount);
		geoData.colors.resize(vertexCount, color);
		if (indexCount > 65535)
			geoData.indices32.resize(indexCount);
		else
			geoData.indices16.resize(indexCount);

		uint32_t vIndex = 0, iIndex = 0;

		float phi = 0.0f, theta = 0.0f;
		float per_phi = PI / levels;
		float per_theta = 2 * PI / slices;
		float x, y, z;

		// 放入顶端点
		geoData.vertices[vIndex] = XMFLOAT3(0.0f, radius, 0.0f);
		geoData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
		geoData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(0.0f, 0.0f);


		for (uint32_t i = 1; i < levels; ++i)
		{
			phi = per_phi * i;
			// 需要slices + 1个顶点是因为 起点和终点需为同一点，但纹理坐标值不一致
			for (uint32_t j = 0; j <= slices; ++j)
			{
				theta = per_theta * j;
				x = radius * sinf(phi) * cosf(theta);
				y = radius * cosf(phi);
				z = radius * sinf(phi) * sinf(theta);
				// 计算出局部坐标、法向量、Tangent向量和纹理坐标
				XMFLOAT3 pos = XMFLOAT3(x, y, z);

				geoData.vertices[vIndex] = pos;
				XMStoreFloat3(&geoData.normals[vIndex], XMVector3Normalize(XMLoadFloat3(&pos)));
				geoData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
				geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(theta / 2 / PI, phi / PI);
			}
		}

		// 放入底端点
		geoData.vertices[vIndex] = XMFLOAT3(0.0f, -radius, 0.0f);
		geoData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
		geoData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
		geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(0.0f, 1.0f);


		// 放入索引
		if (levels > 1)
		{
			for (uint32_t j = 1; j <= slices; ++j)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = 0;
					geoData.indices32[iIndex++] = j % (slices + 1) + 1;
					geoData.indices32[iIndex++] = j;
				}
				else
				{
					geoData.indices16[iIndex++] = 0;
					geoData.indices16[iIndex++] = j % (slices + 1) + 1;
					geoData.indices16[iIndex++] = j;
				}
			}
		}


		for (uint32_t i = 1; i < levels - 1; ++i)
		{
			for (uint32_t j = 1; j <= slices; ++j)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = (i - 1) * (slices + 1) + j;
					geoData.indices32[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
					geoData.indices32[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

					geoData.indices32[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
					geoData.indices32[iIndex++] = i * (slices + 1) + j;
					geoData.indices32[iIndex++] = (i - 1) * (slices + 1) + j;
				}
				else
				{
					geoData.indices16[iIndex++] = (i - 1) * (slices + 1) + j;
					geoData.indices16[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
					geoData.indices16[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

					geoData.indices16[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
					geoData.indices16[iIndex++] = i * (slices + 1) + j;
					geoData.indices16[iIndex++] = (i - 1) * (slices + 1) + j;
				}

			}
		}

		// 逐渐放入索引
		if (levels > 1)
		{
			for (uint32_t j = 1; j <= slices; ++j)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = (levels - 2) * (slices + 1) + j;
					geoData.indices32[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
					geoData.indices32[iIndex++] = (levels - 1) * (slices + 1) + 1;
				}
				else
				{
					geoData.indices16[iIndex++] = (levels - 2) * (slices + 1) + j;
					geoData.indices16[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
					geoData.indices16[iIndex++] = (levels - 1) * (slices + 1) + 1;
				}
			}
		}

		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, vertexCount,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}

	GeometryData CreateSphereEx(float radius, uint32_t levels, uint32_t slices, const DirectX::XMFLOAT4& color, bool bInward,
		float latitudeStartDeg, float latitudeEndDeg, float longitudeRangeDeg)
	{
		using namespace DirectX;

		GeometryData geoData;

		levels = std::max(levels, 1u);
		slices = std::max(slices, 1u);
		latitudeStartDeg = std::clamp(latitudeStartDeg, 0.0f, 180.0f);
		latitudeEndDeg = std::clamp(latitudeEndDeg, 0.0f, 180.0f);
		if (latitudeStartDeg > latitudeEndDeg)
			std::swap(latitudeStartDeg, latitudeEndDeg);
		longitudeRangeDeg = std::clamp(longitudeRangeDeg, 0.0f, 360.0f);

		const float phiStart = latitudeStartDeg / 180.0f * PI;
		const float phiRange = (latitudeEndDeg - latitudeStartDeg) / 180.0f * PI;
		const float thetaStart = 0.0f;
		const float thetaRange = longitudeRangeDeg / 180.0f * PI;
		const float perPhi = phiRange / levels;
		const float perTheta = thetaRange / slices;
		const float invPhiRange = phiRange > 1e-6f ? 1.0f / phiRange : 0.0f;
		const float invThetaRange = thetaRange > 1e-6f ? 1.0f / thetaRange : 0.0f;

		// 生成球面网格顶点与纹理坐标
		auto AddPoint = [&](int i, int j) -> SGeometryPoint {
			SGeometryPoint node;
			float phi = phiStart + perPhi * i;
			float theta = thetaStart + perTheta * j;
			float x = radius * sinf(phi) * cosf(theta);
			float y = radius * cosf(phi);
			float z = radius * sinf(phi) * sinf(theta);
			node.vertice = XMFLOAT3(x, y, z);
			node.ptOutView = CPointD((theta - thetaStart) * invThetaRange, (phi - phiStart) * invPhiRange);
			return node;
		};

		// 由四边形顶点计算面法向（与旋转体/柱面一致，保证法向与平面垂直）
		auto CalcQuadFaceNormal = [&](const std::vector<SGeometryPoint>& quadPts) -> XMVECTOR {
			auto TryCross = [](const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c, const XMFLOAT3& d) -> XMVECTOR {
				XMVECTOR v1 = XMVectorSubtract(XMLoadFloat3(&b), XMLoadFloat3(&a));
				XMVECTOR v2 = XMVectorSubtract(XMLoadFloat3(&d), XMLoadFloat3(&a));
				return XMVector3Cross(v1, v2);
			};
			XMVECTOR faceNormal = TryCross(quadPts[0].vertice, quadPts[1].vertice, quadPts[2].vertice, quadPts[3].vertice);
			float faceNormalLen = XMVectorGetX(XMVector3Length(faceNormal));
			if (faceNormalLen < 1e-6f)
			{
				// 北极行上缘退化为点时，改用 pole→下环 的两条边求法向
				faceNormal = TryCross(quadPts[0].vertice, quadPts[2].vertice, quadPts[2].vertice, quadPts[3].vertice);
				faceNormalLen = XMVectorGetX(XMVector3Length(faceNormal));
			}
			if (faceNormalLen < 1e-6f)
			{
				// 南极行下缘退化为点时，改用上环→pole 的两条边求法向
				faceNormal = TryCross(quadPts[0].vertice, quadPts[1].vertice, quadPts[2].vertice, quadPts[2].vertice);
				faceNormalLen = XMVectorGetX(XMVector3Length(faceNormal));
			}
			if (faceNormalLen < 1e-6f)
				return XMVectorZero();
			return XMVector3Normalize(faceNormal);
		};

		for (uint32_t i = 0; i < levels; ++i)
		{
			for (uint32_t j = 0; j < slices; ++j)
			{
				std::vector<SGeometryPoint> vecPoints;
				vecPoints.push_back(AddPoint(i, j));
				vecPoints.push_back(AddPoint(i, j + 1));
				vecPoints.push_back(AddPoint(i + 1, j + 1));
				vecPoints.push_back(AddPoint(i + 1, j));

				// 面中心径向，用于校正法向朝向
				XMFLOAT3 centerPos;
				XMStoreFloat3(&centerPos, XMVectorScale(
					XMVectorAdd(
						XMVectorAdd(XMLoadFloat3(&vecPoints[0].vertice), XMLoadFloat3(&vecPoints[1].vertice)),
						XMVectorAdd(XMLoadFloat3(&vecPoints[2].vertice), XMLoadFloat3(&vecPoints[3].vertice))),
					0.25f));
				XMVECTOR radial = XMVector3Normalize(XMLoadFloat3(&centerPos));

				XMVECTOR faceNormal = CalcQuadFaceNormal(vecPoints);
				if (XMVectorGetX(XMVector3LengthSq(faceNormal)) < 1e-12f)
					faceNormal = radial;
				else if (XMVectorGetX(XMVector3Dot(faceNormal, radial)) < 0.0f)
					faceNormal = XMVectorNegate(faceNormal);
				if (bInward)
					faceNormal = XMVectorNegate(faceNormal);

				XMFLOAT3 normalFloat;
				XMStoreFloat3(&normalFloat, faceNormal);

				// 切向量沿纬向边（取较长边，避免退化边）
				XMVECTOR edge0 = XMVectorSubtract(XMLoadFloat3(&vecPoints[1].vertice), XMLoadFloat3(&vecPoints[0].vertice));
				XMVECTOR edge1 = XMVectorSubtract(XMLoadFloat3(&vecPoints[3].vertice), XMLoadFloat3(&vecPoints[0].vertice));
				XMVECTOR tangentVec = XMVectorGetX(XMVector3LengthSq(edge0)) >= XMVectorGetX(XMVector3LengthSq(edge1)) ? edge0 : edge1;
				XMFLOAT3 tangentFloat;
				XMStoreFloat3(&tangentFloat, XMVector3Normalize(tangentVec));
				XMFLOAT4 tangent(tangentFloat.x, tangentFloat.y, tangentFloat.z, 1.0f);

				for (auto& pt : vecPoints)
				{
					pt.normal = normalFloat;
					pt.tangent = tangent;
				}

				SSegmentUnitData sPreUnitNode;
				sPreUnitNode.vecSegPoints = vecPoints;
				geoData.vecSegNodes.push_back(sPreUnitNode);
			}
		}

		geoData.UpdateverticeDatas();

		//DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, vertexCount,
		//	(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}

	SGeometryRenderData CreateBox(float width, float height, float depth, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		SGeometryRenderData geoData;

		geoData.vertices.resize(24);
		geoData.normals.resize(24);
		geoData.tangents.resize(24);
		geoData.texcoordArrays.resize(1);
		geoData.texcoordArrays[0].resize(24);
		geoData.colors.resize(24, color);

		float w2 = width / 2, h2 = height / 2, d2 = depth / 2;

		// 右面(+X面)
		geoData.vertices[0] = XMFLOAT3(w2, -h2, -d2);
		geoData.vertices[1] = XMFLOAT3(w2, h2, -d2);
		geoData.vertices[2] = XMFLOAT3(w2, h2, d2);
		geoData.vertices[3] = XMFLOAT3(w2, -h2, d2);
		// 左面(-X面)
		geoData.vertices[4] = XMFLOAT3(-w2, -h2, d2);
		geoData.vertices[5] = XMFLOAT3(-w2, h2, d2);
		geoData.vertices[6] = XMFLOAT3(-w2, h2, -d2);
		geoData.vertices[7] = XMFLOAT3(-w2, -h2, -d2);
		// 顶面(+Y面)
		geoData.vertices[8] = XMFLOAT3(-w2, h2, -d2);
		geoData.vertices[9] = XMFLOAT3(-w2, h2, d2);
		geoData.vertices[10] = XMFLOAT3(w2, h2, d2);
		geoData.vertices[11] = XMFLOAT3(w2, h2, -d2);
		// 底面(-Y面)
		geoData.vertices[12] = XMFLOAT3(w2, -h2, -d2);
		geoData.vertices[13] = XMFLOAT3(w2, -h2, d2);
		geoData.vertices[14] = XMFLOAT3(-w2, -h2, d2);
		geoData.vertices[15] = XMFLOAT3(-w2, -h2, -d2);
		// 背面(+Z面)
		geoData.vertices[16] = XMFLOAT3(w2, -h2, d2);
		geoData.vertices[17] = XMFLOAT3(w2, h2, d2);
		geoData.vertices[18] = XMFLOAT3(-w2, h2, d2);
		geoData.vertices[19] = XMFLOAT3(-w2, -h2, d2);
		// 正面(-Z面)
		geoData.vertices[20] = XMFLOAT3(-w2, -h2, -d2);
		geoData.vertices[21] = XMFLOAT3(-w2, h2, -d2);
		geoData.vertices[22] = XMFLOAT3(w2, h2, -d2);
		geoData.vertices[23] = XMFLOAT3(w2, -h2, -d2);

		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, 24,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		for (size_t i = 0; i < 4; ++i)
		{
			// 右面(+X面)
			geoData.normals[i] = XMFLOAT3(1.0f, 0.0f, 0.0f);
			geoData.tangents[i] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
			// 左面(-X面)
			geoData.normals[i + 4] = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			geoData.tangents[i + 4] = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
			// 顶面(+Y面)
			geoData.normals[i + 8] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			geoData.tangents[i + 8] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			// 底面(-Y面)
			geoData.normals[i + 12] = XMFLOAT3(0.0f, -1.0f, 0.0f);
			geoData.tangents[i + 12] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			// 背面(+Z面)
			geoData.normals[i + 16] = XMFLOAT3(0.0f, 0.0f, 1.0f);
			geoData.tangents[i + 16] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			// 正面(-Z面)
			geoData.normals[i + 20] = XMFLOAT3(0.0f, 0.0f, -1.0f);
			geoData.tangents[i + 20] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		}

		for (size_t i = 0; i < 6; ++i)
		{
			geoData.texcoordArrays[0][i * 4] = XMFLOAT2(0.0f, 1.0f);
			geoData.texcoordArrays[0][i * 4 + 1] = XMFLOAT2(0.0f, 0.0f);
			geoData.texcoordArrays[0][i * 4 + 2] = XMFLOAT2(1.0f, 0.0f);
			geoData.texcoordArrays[0][i * 4 + 3] = XMFLOAT2(1.0f, 1.0f);
		}

		geoData.indices16.resize(36);

		uint16_t indices[] = {
			0, 1, 2, 2, 3, 0,		// 右面(+X面)
			4, 5, 6, 6, 7, 4,		// 左面(-X面)
			8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
			12, 13, 14, 14, 15, 12,	// 底面(-Y面)
			16, 17, 18, 18, 19, 16, // 背面(+Z面)
			20, 21, 22, 22, 23, 20	// 正面(-Z面)
		};
		memcpy_s(geoData.indices16.data(), sizeof indices, indices, sizeof indices);

		return geoData;
	}

	SGeometryRenderData CreateBoxInward(float width, float height, float depth, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		SGeometryRenderData geoData;

		geoData.vertices.resize(24);
		geoData.normals.resize(24);
		geoData.tangents.resize(24);
		geoData.texcoordArrays.resize(1);
		geoData.texcoordArrays[0].resize(24);
		geoData.colors.resize(24, color);

		float w2 = width / 2, h2 = height / 2, d2 = depth / 2;

		// 右面(+X面)
		geoData.vertices[0] = XMFLOAT3(w2, -h2, -d2);
		geoData.vertices[3] = XMFLOAT3(w2, h2, -d2);
		geoData.vertices[2] = XMFLOAT3(w2, h2, d2);
		geoData.vertices[1] = XMFLOAT3(w2, -h2, d2);
		// 左面(-X面)
		geoData.vertices[4] = XMFLOAT3(-w2, -h2, d2);
		geoData.vertices[7] = XMFLOAT3(-w2, h2, d2);
		geoData.vertices[6] = XMFLOAT3(-w2, h2, -d2);
		geoData.vertices[5] = XMFLOAT3(-w2, -h2, -d2);
		// 顶面(+Y面)
		geoData.vertices[8] = XMFLOAT3(-w2, h2, -d2);
		geoData.vertices[11] = XMFLOAT3(-w2, h2, d2);
		geoData.vertices[10] = XMFLOAT3(w2, h2, d2);
		geoData.vertices[9] = XMFLOAT3(w2, h2, -d2);
		// 底面(-Y面)
		geoData.vertices[12] = XMFLOAT3(w2, -h2, -d2);
		geoData.vertices[15] = XMFLOAT3(w2, -h2, d2);
		geoData.vertices[14] = XMFLOAT3(-w2, -h2, d2);
		geoData.vertices[13] = XMFLOAT3(-w2, -h2, -d2);
		// 背面(+Z面)
		geoData.vertices[16] = XMFLOAT3(w2, -h2, d2);
		geoData.vertices[19] = XMFLOAT3(w2, h2, d2);
		geoData.vertices[18] = XMFLOAT3(-w2, h2, d2);
		geoData.vertices[17] = XMFLOAT3(-w2, -h2, d2);
		// 正面(-Z面)
		geoData.vertices[20] = XMFLOAT3(-w2, -h2, -d2);
		geoData.vertices[23] = XMFLOAT3(-w2, h2, -d2);
		geoData.vertices[22] = XMFLOAT3(w2, h2, -d2);
		geoData.vertices[21] = XMFLOAT3(w2, -h2, -d2);

		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, 24,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		for (size_t i = 0; i < 4; ++i)
		{
			// 右面(+X面)
			geoData.normals[i] = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			//geoData.tangents[i] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
			// 左面(-X面)
			geoData.normals[i + 4] = XMFLOAT3(1.0f, 0.0f, 0.0f);
			//geoData.tangents[i + 4] = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
			// 顶面(+Y面)
			geoData.normals[i + 8] = XMFLOAT3(0.0f, -1.0f, 0.0f);
			//geoData.tangents[i + 8] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			// 底面(-Y面)
			geoData.normals[i + 12] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			//geoData.tangents[i + 12] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			// 背面(+Z面)
			geoData.normals[i + 16] = XMFLOAT3(0.0f, 0.0f, -1.0f);
			//geoData.tangents[i + 16] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			// 正面(-Z面)
			geoData.normals[i + 20] = XMFLOAT3(0.0f, 0.0f, 1.0f);
			//geoData.tangents[i + 20] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		}

		for (size_t i = 0; i < 6; ++i)
		{
			geoData.texcoordArrays[0][i * 4] = XMFLOAT2(0.0f, 1.0f);
			geoData.texcoordArrays[0][i * 4 + 3] = XMFLOAT2(0.0f, 0.0f);
			geoData.texcoordArrays[0][i * 4 + 2] = XMFLOAT2(1.0f, 0.0f);
			geoData.texcoordArrays[0][i * 4 + 1] = XMFLOAT2(1.0f, 1.0f);
		}

		geoData.indices16.resize(36);

		uint16_t indices[] = {
			0, 1, 2, 2, 3, 0,		// 右面(+X面)
			4, 5, 6, 6, 7, 4,		// 左面(-X面)
			8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
			12, 13, 14, 14, 15, 12,	// 底面(-Y面)
			16, 17, 18, 18, 19, 16, // 背面(+Z面)
			20, 21, 22, 22, 23, 20	// 正面(-Z面)
		};
		memcpy_s(geoData.indices16.data(), sizeof indices, indices, sizeof indices);

		return geoData;
	}

	SGeometryRenderData CreateCylinder(float radius, float height, uint32_t slices, uint32_t stacks, float texU, float texV, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		SGeometryRenderData geoData;
		uint32_t vertexCount = (slices + 1) * (stacks + 3) + 2;
		uint32_t indexCount = 6 * slices * (stacks + 1);

		geoData.vertices.resize(vertexCount);
		geoData.normals.resize(vertexCount);
		geoData.tangents.resize(vertexCount);
		geoData.texcoordArrays.resize(1);
		geoData.texcoordArrays[0].resize(vertexCount);
		geoData.colors.resize(vertexCount, color);

		if (indexCount > 65535)
			geoData.indices32.resize(indexCount);
		else
			geoData.indices16.resize(indexCount);

		float h2 = height / 2;
		float theta = 0.0f;
		float per_theta = 2 * PI / slices;
		float stackHeight = height / stacks;
		//
		// 侧面部分
		//
		{
			// 自底向上铺设侧面端点
			size_t vIndex = 0;
			for (size_t i = 0; i < stacks + 1; ++i)
			{
				float y = -h2 + i * stackHeight;
				// 当前层顶点
				for (size_t j = 0; j <= slices; ++j)
				{
					theta = j * per_theta;
					float u = theta / 2 / PI;
					float v = 1.0f - (float)i / stacks;

					geoData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), y, radius * sinf(theta)), XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
					geoData.normals[vIndex] = XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
					geoData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
					geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(u * texU, v * texV);
				}
			}

			// 放入索引
			size_t iIndex = 0;
			for (uint32_t i = 0; i < stacks; ++i)
			{
				for (uint32_t j = 0; j < slices; ++j)
				{
					if (indexCount > 65535)
					{
						geoData.indices32[iIndex++] = i * (slices + 1) + j;
						geoData.indices32[iIndex++] = (i + 1) * (slices + 1) + j;
						geoData.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;

						geoData.indices32[iIndex++] = i * (slices + 1) + j;
						geoData.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;
						geoData.indices32[iIndex++] = i * (slices + 1) + j + 1;
					}
					else
					{
						geoData.indices16[iIndex++] = i * (slices + 1) + j;
						geoData.indices16[iIndex++] = (i + 1) * (slices + 1) + j;
						geoData.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;

						geoData.indices16[iIndex++] = i * (slices + 1) + j;
						geoData.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;
						geoData.indices16[iIndex++] = i * (slices + 1) + j + 1;
					}
				}
			}
		}

		//
		// 顶盖底盖部分
		//
		{
			size_t vIndex = (slices + 1) * (stacks + 1), iIndex = 6 * slices * stacks;
			uint32_t offset = static_cast<uint32_t>(vIndex);

			// 放入顶端圆心
			geoData.vertices[vIndex] = XMFLOAT3(0.0f, h2, 0.0f);
			geoData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			geoData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(0.5f, 0.5f);

			// 放入顶端圆上各点
			for (uint32_t i = 0; i <= slices; ++i)
			{
				theta = i * per_theta;
				float u = cosf(theta) * radius / height + 0.5f;
				float v = sinf(theta) * radius / height + 0.5f;
				geoData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), h2, radius * sinf(theta));
				geoData.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
				geoData.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
				geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(u, v);
			}

			// 放入底端圆心
			geoData.vertices[vIndex] = XMFLOAT3(0.0f, -h2, 0.0f);
			geoData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
			geoData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(0.5f, 0.5f);

			// 放入底部圆上各点
			for (uint32_t i = 0; i <= slices; ++i)
			{
				theta = i * per_theta;
				float u = cosf(theta) * radius / height + 0.5f;
				float v = sinf(theta) * radius / height + 0.5f;
				geoData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta));
				geoData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
				geoData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
				geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(u, v);
			}


			// 放入顶部三角形索引
			for (uint32_t i = 1; i <= slices; ++i)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = offset;
					geoData.indices32[iIndex++] = offset + i % (slices + 1) + 1;
					geoData.indices32[iIndex++] = offset + i;
				}
				else
				{
					geoData.indices16[iIndex++] = offset;
					geoData.indices16[iIndex++] = offset + i % (slices + 1) + 1;
					geoData.indices16[iIndex++] = offset + i;
				}

			}

			// 放入底部三角形索引
			offset += slices + 2;
			for (uint32_t i = 1; i <= slices; ++i)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = offset;
					geoData.indices32[iIndex++] = offset + i;
					geoData.indices32[iIndex++] = offset + i % (slices + 1) + 1;
				}
				else
				{
					geoData.indices16[iIndex++] = offset;
					geoData.indices16[iIndex++] = offset + i;
					geoData.indices16[iIndex++] = offset + i % (slices + 1) + 1;
				}
			}
		}

		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, vertexCount,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}

	SGeometryRenderData CreateCirque(float radius, float height, uint32_t slices, uint32_t stacks, float texU, float texV, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		SGeometryRenderData geoData;
		uint32_t vertexCount = (slices + 1) * (stacks + 1);
		uint32_t indexCount = 6 * slices * stacks;

		geoData.vertices.resize(vertexCount);
		geoData.normals.resize(vertexCount);
		geoData.tangents.resize(vertexCount);
		geoData.texcoordArrays.resize(1);
		geoData.texcoordArrays[0].resize(vertexCount);
		geoData.colors.resize(vertexCount, color);

		if (indexCount > 65535)
			geoData.indices32.resize(indexCount);
		else
			geoData.indices16.resize(indexCount);

		float h2 = height / 2;
		float theta = 0.0f;
		float per_theta = 2 * PI / slices;
		float stackHeight = height / stacks;
		//
		// 侧面部分
		//
		{
			// 自底向上铺设侧面端点
			size_t vIndex = 0;
			for (size_t i = 0; i < stacks + 1; ++i)
			{
				float y = -h2 + i * stackHeight;
				// 当前层顶点
				for (size_t j = 0; j <= slices; ++j)
				{
					theta = j * per_theta;
					float u = theta / 2 / PI;
					float v = 1.0f - (float)i / stacks;

					geoData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), y, radius * sinf(theta)), XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
					geoData.normals[vIndex] = XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
					geoData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
					geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(u * texU, v * texV);
				}
			}

			// 放入索引
			size_t iIndex = 0;
			for (uint32_t i = 0; i < stacks; ++i)
			{
				for (uint32_t j = 0; j < slices; ++j)
				{
					if (indexCount > 65535)
					{
						geoData.indices32[iIndex++] = i * (slices + 1) + j;
						geoData.indices32[iIndex++] = (i + 1) * (slices + 1) + j;
						geoData.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;

						geoData.indices32[iIndex++] = i * (slices + 1) + j;
						geoData.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;
						geoData.indices32[iIndex++] = i * (slices + 1) + j + 1;
					}
					else
					{
						geoData.indices16[iIndex++] = i * (slices + 1) + j;
						geoData.indices16[iIndex++] = (i + 1) * (slices + 1) + j;
						geoData.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;

						geoData.indices16[iIndex++] = i * (slices + 1) + j;
						geoData.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;
						geoData.indices16[iIndex++] = i * (slices + 1) + j + 1;
					}
				}
			}
		}

		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, vertexCount,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}

	SGeometryRenderData CreateCone(float radius, float height, uint32_t slices, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		SGeometryRenderData geoData;

		uint32_t vertexCount = 3 * slices + 1;
		uint32_t indexCount = 6 * slices;
		geoData.vertices.resize(vertexCount);
		geoData.normals.resize(vertexCount);
		geoData.tangents.resize(vertexCount);
		geoData.texcoordArrays.resize(1);
		geoData.texcoordArrays[0].resize(vertexCount);
		geoData.colors.resize(vertexCount, color);

		if (indexCount > 65535)
			geoData.indices32.resize(indexCount);
		else
			geoData.indices16.resize(indexCount);

		float h2 = height / 2;
		float theta = 0.0f;
		float per_theta = 2 * PI / slices;
		float len = sqrtf(height * height + radius * radius);

		//
		// 圆锥侧面
		//
		{
			size_t iIndex = 0;
			size_t vIndex = 0;

			// 放入圆锥尖端顶点(每个顶点位置相同，但包含不同的法向量和切线向量)
			for (uint32_t i = 0; i < slices; ++i)
			{
				theta = i * per_theta + per_theta / 2;
				geoData.vertices[vIndex] = XMFLOAT3(0.0f, h2, 0.0f);
				geoData.normals[vIndex] = XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len);
				geoData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
				geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(0.5f, 0.5f);
			}

			// 放入圆锥侧面底部顶点
			for (uint32_t i = 0; i < slices; ++i)
			{
				theta = i * per_theta;
				geoData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta));
				geoData.normals[vIndex] = XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len);
				geoData.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
				geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f);
			}

			// 放入索引
			for (uint32_t i = 0; i < slices; ++i)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = i;
					geoData.indices32[iIndex++] = slices + (i + 1) % slices;
					geoData.indices32[iIndex++] = slices + i % slices;
				}
				else
				{
					geoData.indices16[iIndex++] = i;
					geoData.indices16[iIndex++] = slices + (i + 1) % slices;
					geoData.indices16[iIndex++] = slices + i % slices;
				}
			}
		}

		//
		// 圆锥底面
		//
		{
			size_t iIndex = 3 * (size_t)slices;
			size_t vIndex = 2 * (size_t)slices;

			// 放入圆锥底面顶点
			for (uint32_t i = 0; i < slices; ++i)
			{
				theta = i * per_theta;

				geoData.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)),
					geoData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
				geoData.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
				geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f);
			}
			// 放入圆锥底面圆心
			geoData.vertices[vIndex] = XMFLOAT3(0.0f, -h2, 0.0f),
				geoData.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
			geoData.texcoordArrays[0][vIndex++] = XMFLOAT2(0.5f, 0.5f);

			// 放入索引
			uint32_t offset = 2 * slices;
			for (uint32_t i = 0; i < slices; ++i)
			{
				if (indexCount > 65535)
				{
					geoData.indices32[iIndex++] = offset + slices;
					geoData.indices32[iIndex++] = offset + i % slices;
					geoData.indices32[iIndex++] = offset + (i + 1) % slices;
				}
				else
				{
					geoData.indices16[iIndex++] = offset + slices;
					geoData.indices16[iIndex++] = offset + i % slices;
					geoData.indices16[iIndex++] = offset + (i + 1) % slices;
				}

			}
		}

		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, vertexCount,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}

	GeometryData CreatePlane(const DirectX::XMFLOAT2& planeSize, const DirectX::XMFLOAT2& maxTexCoord)
	{
		return CreatePlane(planeSize.x, planeSize.y, maxTexCoord.x, maxTexCoord.y);
	}

	GeometryData CreatePlane(float width, float depth, float texU, float texV, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		GeometryData geoData;

		SGeometryPoint p1;
		uint32_t vIndex = 0;
		p1.vertice = XMFLOAT3(-width / 2, 0.0f, -depth / 2);
		p1.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		p1.tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		p1.ptOutView = CPointD(0.0f, texV);
		p1.color = color;

		SGeometryPoint p2;
		p2.vertice = XMFLOAT3(-width / 2, 0.0f, depth / 2);
		p2.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		p2.tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		p2.ptOutView = CPointD(0.0f, 0.0f);
		p2.color = color;

		SGeometryPoint p3;
		p3.vertice = XMFLOAT3(width / 2, 0.0f, depth / 2);
		p3.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		p3.tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		p3.ptOutView = CPointD(texU, 0.0f);
		p3.color = color;

		SGeometryPoint p4;
		p4.vertice = XMFLOAT3(width / 2, 0.0f, -depth / 2);
		p4.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		p4.tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		p4.ptOutView = CPointD(texU, texV);
		p4.color = color;

		// 预览单元数据
		SSegmentUnitData sPreUnitNode;
		sPreUnitNode.vecSegPoints.push_back(p1);
		sPreUnitNode.vecSegPoints.push_back(p2);
		sPreUnitNode.vecSegPoints.push_back(p3);
		sPreUnitNode.vecSegPoints.push_back(p4);

		geoData.vecSegNodes.push_back(sPreUnitNode);
		geoData.UpdateverticeDatas();

		return geoData;
	}

	GeometryData CreateGrid(const DirectX::XMFLOAT2& gridSize, const DirectX::XMUINT2& slices, const DirectX::XMFLOAT2& maxTexCoord,
		const std::function<float(float, float)>& heightFunc,
		const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc,
		const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc)
	{
		using namespace DirectX;

		GeometryData geoData;

		float sliceWidth = gridSize.x / slices.x;
		float sliceDepth = gridSize.y / slices.y;
		float leftBottomX = -gridSize.x / 2;
		float leftBottomZ = -gridSize.y / 2;
		float sliceTexWidth = maxTexCoord.x / slices.x;
		float sliceTexDepth = maxTexCoord.y / slices.y;


		//qDebug() << "----------sliceTexWidth: " << sliceTexWidth << "sliceTexDepth " << sliceTexDepth;

		// 创建网格顶点
		//  __ __
		// | /| /|
		// |/_|/_|
		// | /| /| 
		// |/_|/_|

		auto AddPoint = [&](int z, int x, bool bPre) -> SGeometryPoint {

			SGeometryPoint point;

			float posZ = leftBottomZ + z * sliceDepth;
			float posX = leftBottomX + x * sliceWidth;
			// 计算法向量并归一化
			XMFLOAT3 normal = normalFunc(posX, posZ);
			XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&normal)));

			XMFLOAT4 tangent;
			// 计算法平面与z=posZ平面构成的直线单位切向量，维持w分量为1.0f
			XMStoreFloat4(&tangent, XMVector3Normalize(XMVectorSet(normal.y, -normal.x, 0.0f, 0.0f)) + g_XMIdentityR3);

			point.vertice = XMFLOAT3(posX, heightFunc(posX, posZ), posZ);
			point.normal = normal;
			point.tangent = tangent;
			point.ptOutView = CPointD(x * sliceTexWidth, maxTexCoord.y - z * sliceTexDepth);

			return point;
		};

		for (uint32_t z = 0; z < slices.y; ++z)
		{
			for (uint32_t x = 0; x < slices.x; ++x)
			{
				SSegmentUnitData sPreUnitNode;

				std::vector<SGeometryPoint> vec;

				sPreUnitNode.vecSegPoints.push_back(AddPoint(z, x, false));
				sPreUnitNode.vecSegPoints.push_back(AddPoint(z, x + 1, false));
				sPreUnitNode.vecSegPoints.push_back(AddPoint(z + 1, x + 1, false));
				sPreUnitNode.vecSegPoints.push_back(AddPoint(z + 1, x, false));



				geoData.vecSegNodes.push_back(sPreUnitNode);
			}
		}

		geoData.UpdateverticeDatas();

		return geoData;
	}

	SGeometryRenderData CreateTorus(float innerRadius, float outerRadius, uint32_t radialSegments, uint32_t tubularSegments, const DirectX::XMFLOAT4& color)
	{
		SGeometryRenderData geoData;

		// 计算总顶点数
		uint32_t totalVertices = radialSegments * tubularSegments;

		// 预留内存空间以提高性能
		geoData.vertices.resize(totalVertices);
		geoData.normals.resize(totalVertices);
		geoData.texcoordArrays.resize(1); // 添加一个纹理坐标数组
		geoData.texcoordArrays[0].resize(totalVertices);
		geoData.tangents.resize(totalVertices);
		geoData.bitangents.resize(totalVertices);
		geoData.colors.resize(totalVertices, color);

		// 计算索引数量并预留空间
		uint32_t totalIndices = radialSegments * tubularSegments * 6; // 每个四边形两个三角形，每个三角形3个索引
		if (totalVertices > 65535)
			geoData.indices32.reserve(totalIndices);
		else
			geoData.indices16.reserve(totalIndices);

		float ringRadius = (outerRadius - innerRadius) / 2.0f;
		float torusCenterRadius = innerRadius + ringRadius;

		size_t vIndex = 0;
		// 生成顶点、法线、纹理坐标、切线、双切线和颜色
		for (uint32_t i = 0; i < radialSegments; ++i)
		{
			float u = static_cast<float>(i) / radialSegments;
			float theta = u * DirectX::XM_2PI;  // 圆环主圆的角度

			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);

			DirectX::XMFLOAT3 center(
				torusCenterRadius * cosTheta,
				0.0f,
				torusCenterRadius * sinTheta
			);

			for (uint32_t j = 0; j < tubularSegments; ++j)
			{
				float v = static_cast<float>(j) / tubularSegments;
				float phi = v * DirectX::XM_2PI;  // 管道小圆的角度

				float cosPhi = cosf(phi);
				float sinPhi = sinf(phi);

				float x = ringRadius * cosPhi * cosTheta;
				float y = ringRadius * sinPhi;
				float z = ringRadius * cosPhi * sinTheta;

				// 顶点位置
				geoData.vertices[vIndex] = DirectX::XMFLOAT3(center.x + x, y, center.z + z);

				// 法线向量（从圆环中心指向顶点方向，并归一化）
				DirectX::XMFLOAT3 normal(x, y, z);
				DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal));
				DirectX::XMStoreFloat3(&normal, normalVec);
				geoData.normals[vIndex] = normal;

				// 纹理坐标
				geoData.texcoordArrays[0][vIndex] = DirectX::XMFLOAT2(u, v);

				// 切线向量（沿着主圆的切线方向）
				DirectX::XMFLOAT4 tangent(-sinTheta, 0.0f, cosTheta, 0.0f);
				geoData.tangents[vIndex] = tangent;

				// 双切线向量（沿着管道小圆的切线方向）
				DirectX::XMFLOAT4 bitangent(cosPhi * cosTheta, cosPhi * sinTheta, -sinPhi, 0.0f);
				geoData.bitangents[vIndex++] = bitangent;

			}
		}

		// 生成索引（两组三角形组成一个四边形）
		for (uint32_t i = 0; i < radialSegments; ++i)
		{
			for (uint32_t j = 0; j < tubularSegments; ++j)
			{
				uint32_t first = (i * tubularSegments) + j;
				uint32_t second = (((i + 1) % radialSegments) * tubularSegments) + j;

				uint32_t first_next = first + 1;
				uint32_t second_next = second + 1;

				if (j == tubularSegments - 1)
				{
					first_next = i * tubularSegments;
					second_next = ((i + 1) % radialSegments) * tubularSegments;
				}

				if (totalVertices > 65535)
				{
					// 第一个三角形
					geoData.indices32.emplace_back(first);
					geoData.indices32.emplace_back(first_next);
					geoData.indices32.emplace_back(second);

					// 第二个三角形
					geoData.indices32.emplace_back(second);
					geoData.indices32.emplace_back(first_next);
					geoData.indices32.emplace_back(second_next);
				}
				else
				{
					// 第一个三角形
					geoData.indices16.emplace_back(first);
					geoData.indices16.emplace_back(first_next);
					geoData.indices16.emplace_back(second);

					// 第二个三角形
					geoData.indices16.emplace_back(second);
					geoData.indices16.emplace_back(first_next);
					geoData.indices16.emplace_back(second_next);
				}
			}
		}

		// 计算包围盒
		DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, totalVertices,
			(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}


	//耳切法查找第一个耳朵的序号
	int EarClipping(std::vector<int> &polygon, std::vector<CPointD> &vecVertexs, bool bClockwise)
	{
		int nPolygonSize = polygon.size();
		int nVertexNum = vecVertexs.size();
		int prev, next;
		bool earFound = false;
		for (int i = 0; i < nPolygonSize; ++i)
		{
			int prev = polygon[(i - 1 + nPolygonSize) % nPolygonSize];
			int next = polygon[(i + 1) % nPolygonSize];
			int curr = polygon[i];

			CPointD& prevPos = vecVertexs[prev];
			CPointD& currPos = vecVertexs[curr];
			CPointD& nextPos = vecVertexs[next];

			// 步骤 1：当前角必须是凸角
			double dArea = (currPos.x - prevPos.x) * (nextPos.y - prevPos.y) - (currPos.y - prevPos.y) * (nextPos.x - prevPos.x); //有向面积
			if (fabs(dArea) < 1e-8)	//防止共线
				continue;

			bool bIsConvex = bClockwise ? dArea < 0 : dArea> 0;
			if (!bIsConvex)
				continue;

			// 步骤 2：判断三角形内部是否包含其他点
			bool bIsEar = true;
			for (int j = 0; j < nPolygonSize; ++j)
			{
				if (j == i || j == (i - 1 + nVertexNum) % nVertexNum || j == (i + 1) % nVertexNum)
					continue;

				CPointD& testPos = vecVertexs[polygon[j]];
				double a1 = (prevPos.x - testPos.x) * (currPos.y - testPos.y) - (prevPos.y - testPos.y) * (currPos.x - testPos.x);
				double a2 = (currPos.x - testPos.x) * (nextPos.y - testPos.y) - (currPos.y - testPos.y) * (nextPos.x - testPos.x);
				double a3 = (nextPos.x - testPos.x) * (prevPos.y - testPos.y) - (nextPos.y - testPos.y) * (prevPos.x - testPos.x);

				bIsEar = (dArea < 0 || a1 < 0 || a2 < 0 || a3 < 0) && (dArea > 0 || a1 > 0 || a2 > 0 || a3 > 0);
				if (!bIsEar)
					break;
			}

			if (!bIsEar)
				continue;
			else
				return i;
		}

		//没找到
		return -1;
	}

	float CalRCornerRadius(std::vector<CPointD>& vecPoint)
	{
		const int n = static_cast<int>(vecPoint.size());
		if (n < 3)
		{
			return 0.0f;
		}

		constexpr double kEps = 1e-9;
		double minMaxRadius = DBL_MAX;

		std::vector<double> vecCotHalf(n, 0.0);
		std::vector<bool> vecAngleValid(n, false);

		auto CalcVertexHalfTrig = [&](int vtx, double& tanHalf, double& cotHalf) -> bool
		{
			const int prev = (vtx - 1 + n) % n;
			const int next = (vtx + 1) % n;

			const CPointD& prevPos = vecPoint[prev];
			const CPointD& currPos = vecPoint[vtx];
			const CPointD& nextPos = vecPoint[next];

			const double vBeginX = prevPos.x - currPos.x;
			const double vBeginY = prevPos.y - currPos.y;
			const double vEndX = nextPos.x - currPos.x;
			const double vEndY = nextPos.y - currPos.y;

			const double l1 = std::hypot(vBeginX, vBeginY);
			const double l2 = std::hypot(vEndX, vEndY);
			const double dot = vBeginX * vEndX + vBeginY * vEndY;
			const double cross = vBeginX * vEndY - vBeginY * vEndX;
			const double absCross = std::fabs(cross);
			const double denomTan = l1 * l2 + dot;

			if (l1 < kEps || l2 < kEps || denomTan <= kEps || absCross <= kEps)
			{
				return false;
			}

			tanHalf = absCross / denomTan;
			cotHalf = denomTan / absCross;
			return true;
		};

		for (int i = 0; i < n; ++i)
		{
			double tanHalf = 0.0;
			double cotHalf = 0.0;
			if (!CalcVertexHalfTrig(i, tanHalf, cotHalf))
			{
				continue;
			}

			vecCotHalf[i] = cotHalf;
			vecAngleValid[i] = true;

			const int prev = (i - 1 + n) % n;
			const int next = (i + 1) % n;
			const CPointD& prevPos = vecPoint[prev];
			const CPointD& currPos = vecPoint[i];
			const CPointD& nextPos = vecPoint[next];

			const double l1 = std::hypot(currPos.x - prevPos.x, currPos.y - prevPos.y);
			const double l2 = std::hypot(nextPos.x - currPos.x, nextPos.y - currPos.y);
			const double maxRadiusAtVertex = std::min(l1, l2) * tanHalf;
			minMaxRadius = std::min(minMaxRadius, maxRadiusAtVertex);
		}

		for (int i = 0; i < n; ++i)
		{
			const int nextIdx = (i + 1) % n;
			if (!vecAngleValid[i] || !vecAngleValid[nextIdx])
			{
				continue;
			}

			const CPointD& beginPos = vecPoint[i];
			const CPointD& endPos = vecPoint[nextIdx];
			const double edgeLen = std::hypot(endPos.x - beginPos.x, endPos.y - beginPos.y);
			const double cotSum = vecCotHalf[i] + vecCotHalf[nextIdx];

			if (edgeLen < kEps || cotSum <= kEps)
			{
				continue;
			}

			const double maxRadiusAtEdge = edgeLen / cotSum;
			minMaxRadius = std::min(minMaxRadius, maxRadiusAtEdge);
		}

		if (minMaxRadius >= DBL_MAX)
		{
			return 0.0f;
		}

		return static_cast<float>(minMaxRadius);
	}

	GeometryData CreateIrregularPrism(SIrregularPrismBuildParams& buildParams)
	{
		using namespace DirectX;

		auto& vecPoint = buildParams.vecPoint;
		const auto& vecFacePresent = buildParams.vecFacePresent;
		const int nHeight = buildParams.nHeight;
		const bool bClose = buildParams.bClose;
		const bool bConvex = buildParams.bConvex;
		const bool bBottomFace = buildParams.bBottomFace;
		const bool bUpperFace = buildParams.bUpperFace;
		const bool bNormalInward = buildParams.bNormalInward;
		const size_t nBaseSubdivisionAmount = buildParams.nBaseSubdivisionAmount;
		const bool bBottomfaceCorner = buildParams.bBottomfaceCorner;
		const bool bUpperfaceCorner = buildParams.bUpperfaceCorner;
		const int cornerRadius = buildParams.cornerRadius;
		const DirectX::XMFLOAT4& color = buildParams.color;

		int n = vecPoint.size();
		if (n < 2)
			return GeometryData();

		//单位转换 mm -> M 

		//R角相关计算
		double cornerAngle = M_PI_2;	//R角度数
		const double dCornerRadius = cornerRadius / 1000.0;	// R角半径（m）
		double cornerArcLength = dCornerRadius * cornerAngle;	//R角弧长
		
		double cornerTangentOffsetY = dCornerRadius * sin(cornerAngle);			//R角内倒角在y轴相对距离
		int cornerArcSegmentCount = static_cast<int>(nBaseSubdivisionAmount);

		double h = nHeight / 1000.0;
		double h2 = nHeight / 2000.0;
		double middleStraightHeight = nHeight / 1000.0;
		if (bUpperfaceCorner)
			middleStraightHeight -= cornerTangentOffsetY;

		if (bBottomfaceCorner)
			middleStraightHeight -= cornerTangentOffsetY;

		//单位转换过后的顶点坐标
		std::vector<CPointD> vecVertexs;
		vecVertexs.resize(n);

		// 柱面纹理的x轴坐标
		std::vector<double> vecCylinderTextureW;
		vecCylinderTextureW.resize(bClose ? n + 1 : n);

		//柱面柱面纹理的y轴坐标
		double CylinderTextureH1 = 0.0;
		double CylinderTextureH2 = 1.0;
		double ratio = 1.0;	//纹理缩放比例

		bool bClockwise = false;						// 底面形状的点序是顺时针还是逆时针
		double sum = 0.0;								// 面积和
		double totalLength = 0.0;						// 柱面周长

		vecVertexs[0].x = vecPoint[0].x / 1000.0;
		vecVertexs[0].y = vecPoint[0].y / 1000.0;
		vecCylinderTextureW[0] = 0;

		for (int i = 1; i < n; ++i)
		{
			auto& p1 = vecPoint[i];

			vecVertexs[i].x = p1.x / 1000.0f;
			vecVertexs[i].y = p1.y / 1000.0f;

			totalLength += sqrt(pow(vecVertexs[i].x - vecVertexs[i - 1].x, 2) + pow(vecVertexs[i].y - vecVertexs[i - 1].y, 2));
			vecCylinderTextureW[i] = totalLength;

			if (bClose)
			{
				auto& p2 = vecPoint[(i + 1) % n];
				sum += (p2.x - p1.x) * (p2.y + p1.y);  // 等价于 2 * area
			}
		}

		if (bClose)
		{
			sum += (vecPoint[1].x - vecPoint[0].x) * (vecPoint[1].y + vecPoint[0].y);
			bClockwise = sum > 0;

			totalLength += sqrt(pow(vecVertexs[0].x - vecVertexs[n - 1].x, 2) + pow(vecVertexs[0].y - vecVertexs[n - 1].y, 2));
			vecCylinderTextureW[n] = totalLength;
		}

		constexpr double kMinPerimeterLength = 1e-9;
		if (totalLength < kMinPerimeterLength)
		{
			return GeometryData();
		}

		//计算柱面纹理的x坐标
		for (size_t j = 0; j < vecCylinderTextureW.size(); j++)
		{
			vecCylinderTextureW[j] = vecCylinderTextureW[j] / totalLength;
		}

		const int nCylinderTextureCount = static_cast<int>(vecCylinderTextureW.size());
		auto IsCylinderTextureIndexValid = [&](int index) -> bool
		{
			return index >= 0 && index < nCylinderTextureCount;
		};

		//开始创建数据
		GeometryData geoData;

		//一个面的索引顺序
		int arrIndices[6] = { 0, 1, 2, 2, 3, 0 };

		struct SCornerVertexInfo
		{
			SimpleMath::Vector3 internalCylinderCenter;	//倒角圆柱圆心
			SimpleMath::Vector3 internalTangentPt;	//倒角内切点
			SimpleMath::Vector3 middleStraightFacePt;	//中间柱体面轮廓
		};

		struct SVertexInfo
		{
			double vertexAngle;
			SimpleMath::Vector3 edgeNormal;//出边法向量
			XMFLOAT4 edgeTangent;
		};

		std::map <int, SVertexInfo> vertexInfos;
		std::map <int, SCornerVertexInfo> topCornerVertexInfos;
		std::map <int, SCornerVertexInfo> bottomCornerVertexInfos;

		std::vector<CPointD> vecTopFaceVertex;
		std::vector<CPointD> vecBottomFaceVertex;

		//插入柱面数据
		auto InsertCylinderCommonData = [&](CPointD begin, CPointD end, int nBeginIndex, XMFLOAT3 normals, XMFLOAT4 tangents) -> void
		{
			if (!IsCylinderTextureIndexValid(nBeginIndex) || !IsCylinderTextureIndexValid(nBeginIndex + 1))
			{
				return;
			}

			// 预览单元数据
			auto AddPoint = [&](XMFLOAT3 vertice, XMFLOAT2 samplePos, bool bPre)->SGeometryPoint {

				SGeometryPoint node;
				node.vertice = vertice;
				node.ptOutView = CPointD(samplePos.x, samplePos.y);
				node.normal = normals;
				node.tangent = tangents;
				node.color = color;

				return node;
			};

			SSegmentUnitData sPreNode;
		
			double dBeginH = -h2;	// h2：柱体高度的一半
			double dEndH = -h2 + middleStraightHeight;

			if (bBottomfaceCorner)
			{
				dBeginH += cornerTangentOffsetY;
				dEndH += cornerTangentOffsetY;
			}
			
			
			sPreNode.vecSegPoints.push_back(AddPoint(XMFLOAT3(begin.x, dBeginH, begin.y), XMFLOAT2(vecCylinderTextureW[nBeginIndex], CylinderTextureH2), false));
			sPreNode.vecSegPoints.push_back(AddPoint(XMFLOAT3(begin.x, dEndH, begin.y), XMFLOAT2(vecCylinderTextureW[nBeginIndex], CylinderTextureH1), false));
			sPreNode.vecSegPoints.push_back(AddPoint(XMFLOAT3(end.x, dEndH, end.y), XMFLOAT2(vecCylinderTextureW[nBeginIndex + 1], CylinderTextureH1), false));
			sPreNode.vecSegPoints.push_back(AddPoint(XMFLOAT3(end.x, dBeginH, end.y), XMFLOAT2(vecCylinderTextureW[nBeginIndex + 1], CylinderTextureH2), false));

			geoData.vecSegNodes.push_back(sPreNode);


		};

		auto GenerateFaces = [&]()
		{
			auto CalcBox = [&](double& minX, double& maxX, double& minY, double& maxY, const std::vector<CPointD>& vecContourPts)
			{
				for (const auto& pt : vecContourPts)
				{
					minX = std::min(minX, pt.x);
					maxX = std::max(maxX, pt.x);
					minY = std::min(minY, pt.y);
					maxY = std::max(maxY, pt.y);
				}
			};

			// 1. 计算纹理范围
			double minTopX = DBL_MAX;
			double maxTopX = -DBL_MAX;
			double minTopY = DBL_MAX;
			double maxTopY = -DBL_MAX;

			CalcBox(minTopX, maxTopX, minTopY, maxTopY, bUpperfaceCorner ? vecTopFaceVertex : vecVertexs);

			double minBottomX = DBL_MAX;
			double maxBottomX = -DBL_MAX;
			double minBottomY = DBL_MAX;
			double maxBottomY = -DBL_MAX;

			CalcBox(minBottomX, maxBottomX, minBottomY, maxBottomY, bBottomfaceCorner ? vecBottomFaceVertex : vecVertexs);

			double minX = DBL_MAX;
			double maxX = -DBL_MAX;
			double minY = DBL_MAX;
			double maxY = -DBL_MAX;

			minX = std::min(minTopX, minBottomX);
			maxX = std::max(maxTopX, maxBottomX);
			minY = std::min(minTopY, minBottomY);
			maxY = std::max(maxTopY, maxBottomY);


			double dTopBasesW = maxTopX - minTopX;
			double dTopBasesH = maxTopY - minTopY;

			double dBottomBasesW = maxBottomX - minBottomX;
			double dBottomBasesH = maxBottomY - minBottomY;

			// 2. 纹理缩放比例
			double dTotalTextureH = dTopBasesH + dBottomBasesH + middleStraightHeight;
			if (bBottomfaceCorner)
				dTotalTextureH += cornerArcLength;
			if (bUpperfaceCorner)
				dTotalTextureH += cornerArcLength;

			if (dTotalTextureH > totalLength)
			{
				ratio = 1.0 / dTotalTextureH;

				// 调整柱面纹理的x坐标
				for (size_t j = 0; j < vecCylinderTextureW.size(); j++)
				{
					vecCylinderTextureW[j] = vecCylinderTextureW[j] * ratio * totalLength;
				}
			}
			else
			{
				ratio = 1.0 / totalLength;
			}

			CylinderTextureH1 = dTopBasesH;
			if (bUpperfaceCorner)
				CylinderTextureH1 += cornerArcLength;
			CylinderTextureH1 *= ratio;

			CylinderTextureH2 = dTopBasesH + middleStraightHeight;
			if (bUpperfaceCorner)
				CylinderTextureH2 += cornerArcLength;

			CylinderTextureH2 *= ratio;

			// 3. 直接生成完整多边形面片
			auto GenerateFace = [&](bool bUpper, float height, float normalDir, std::vector<CPointD>& vecFaceVertexs)
			{
				SSegmentUnitData faceData;
				faceData.bConvex = bConvex;

				// 直接遍历所有输入顶点，生成完整多边形

				for (const auto& pt : vecFaceVertexs)
				{
					double x = pt.x;
					double z = pt.y;

					// 纹理坐标
					double Tx = (x - (bUpper ? minTopX : minBottomX)) * ratio;
					double Tz = (z - (bUpper ? minTopY : minBottomY)) * ratio;

					if (!bUpper)
						Tz += CylinderTextureH2 + (bBottomfaceCorner ? cornerArcLength * ratio : 0.0);

					// 构造顶点数据
					SGeometryPoint point;
					point.vertice = XMFLOAT3(x, height, z);
					point.normal = XMFLOAT3(0, normalDir, 0);
					point.ptOutView = CPointD(Tx, Tz);
					point.tangent = XMFLOAT4(-1.0f, 0.0f, height, 1.0f);
					point.color = color;

					faceData.vecSegPoints.push_back(point);
				}

				if (!faceData.vecSegPoints.empty())
				{
					geoData.vecSegNodes.push_back(faceData);
				}
			};

			// 法向
			float nNormals = bNormalInward ? 1.0f : -1.0f;

			// 顶面生成
			if (bUpperFace)
			{
				auto& vertices = bUpperfaceCorner ? vecTopFaceVertex : vecVertexs;
				GenerateFace(true, h2, -nNormals, vertices);
			}

			// 底面生成
			if (bBottomFace)
			{
				auto& vertices = bBottomfaceCorner ? vecBottomFaceVertex : vecVertexs;
				GenerateFace(false, -h2, nNormals, vertices);
			}

		};

		auto CreateGeometryPt = [&](bool bUpperCorner)
		{
			for (int i = 0; i < n; ++i)
			{
				int prev = (i - 1 + n) % n;
				int next = (i + 1) % n;
				int curr = i;

				double middleStraightFaceY = bUpperCorner ? (h2 - cornerTangentOffsetY) : -(h2 - cornerTangentOffsetY);

				auto& cornerVertexInfo = bUpperCorner ? topCornerVertexInfos : bottomCornerVertexInfos;
				auto& vecFaceVertex = bUpperCorner ? vecTopFaceVertex : vecBottomFaceVertex;

				CPointD currPos = vecVertexs[curr];
				SimpleMath::Vector3 curr3dPos = SimpleMath::Vector3(currPos.x, middleStraightFaceY, currPos.y);

				CPointD prevPos = vecVertexs[prev];
				CPointD nextPos = vecVertexs[next];

				cornerVertexInfo[curr].middleStraightFacePt = curr3dPos;

				CPointD tempPoint = CalcLineIntersect(DirectX::XMFLOAT2(currPos.x - prevPos.x, currPos.y - prevPos.y),
					DirectX::XMFLOAT2(nextPos.x - currPos.x, nextPos.y - currPos.y),
					CPointD(curr3dPos.x, curr3dPos.z),
					DirectX::XMFLOAT2(-vertexInfos[prev].edgeNormal.x, -vertexInfos[prev].edgeNormal.z),
					DirectX::XMFLOAT2(-vertexInfos[curr].edgeNormal.x, -vertexInfos[curr].edgeNormal.z),
					dCornerRadius);
				cornerVertexInfo[curr].internalCylinderCenter = SimpleMath::Vector3(tempPoint.x, middleStraightFaceY, tempPoint.y);
				vecFaceVertex.push_back(tempPoint);


				tempPoint = MovePointAlongDirection(tempPoint, DirectX::XMFLOAT2(vertexInfos[curr].edgeNormal.x, vertexInfos[curr].edgeNormal.z), dCornerRadius * cos(cornerAngle));
				cornerVertexInfo[curr].internalTangentPt = SimpleMath::Vector3(tempPoint.x, bUpperCorner ? h2 : -h2, tempPoint.y);

				const double vBeginX = prevPos.x - currPos.x;
				const double vBeginY = prevPos.y - currPos.y;
				const double vEndX = nextPos.x - currPos.x;
				const double vEndY = nextPos.y - currPos.y;
				// 两条边 R 角衔接点相对顶点的张角，用于 subTheta 扫描
				vertexInfos[curr].vertexAngle = std::fabs(std::atan2(vBeginX * vEndY - vBeginY * vEndX, vBeginX * vEndX + vBeginY * vEndY));
			}
		};

		auto AddNewEdgeRCorner = [&](bool bTopFace, int curr, int prev, int next, double rCornerAngle) {

			if (!IsCylinderTextureIndexValid(curr) || !IsCylinderTextureIndexValid(curr + 1))
			{
				return;
			}

			auto& cornerVertexInfo = bTopFace ? topCornerVertexInfos : bottomCornerVertexInfos;

			struct SCornerSample
			{
				SimpleMath::Vector3 pos;
				XMFLOAT2 uv;
			};

			auto SampleCorner = [&](int i, int j) -> SCornerSample
			{
				SCornerSample sample;

				double subPhi = rCornerAngle / cornerArcSegmentCount * j;

				SimpleMath::Vector3 localX = vertexInfos[curr].edgeNormal;
				SimpleMath::Vector3 localY = SimpleMath::Vector3(0, 1, 0);
				SimpleMath::Vector3 localZ = SimpleMath::Vector3(vertexInfos[curr].edgeTangent.x, vertexInfos[curr].edgeTangent.y, vertexInfos[curr].edgeTangent.z);
				localZ.Normalize();

				double tanVal = 0.0;
				double h = 0.0;
				if (i == 0)
				{
					tanVal = std::tan(M_PI_2 + vertexInfos[curr].vertexAngle * 0.5);
					h = 0 - tanVal * (-dCornerRadius * cos(cornerAngle));
				}
				else
				{
					tanVal = std::tan(M_PI_2 - vertexInfos[next].vertexAngle * 0.5);
					h = (cornerVertexInfo[next].internalTangentPt - cornerVertexInfo[curr].internalTangentPt).Length()
						- tanVal * (-dCornerRadius * cos(cornerAngle));
				}

				float x = dCornerRadius * std::cos(subPhi);
				float y = dCornerRadius * std::sin(subPhi);
				float z = dCornerRadius * tanVal * cos(subPhi) + h;

				sample.pos = cornerVertexInfo[curr].internalCylinderCenter + localX * x + localY * y + localZ * z;


				SimpleMath::Vector3 prjPos = sample.pos;
				prjPos.y = cornerVertexInfo[(curr + i) % n].middleStraightFacePt.y;

				double offset = (prjPos - cornerVertexInfo[(curr + i) % n].middleStraightFacePt).Length() * cos(vertexInfos[(curr + i) % n].vertexAngle * 0.5);

				const int texIndex = curr + i;
				if (!IsCylinderTextureIndexValid(texIndex))
				{
					return sample;
				}

				if (bTopFace)
				{
					sample.uv = XMFLOAT2{
						(float)(vecCylinderTextureW[texIndex] + (i == 0 ? offset / totalLength : -offset / totalLength)),
						(float)(CylinderTextureH1 - j * cornerArcLength / cornerArcSegmentCount * ratio)
					};
				}
				else
				{
					sample.uv = XMFLOAT2{
						(float)(vecCylinderTextureW[texIndex] + (i == 0 ? offset / totalLength : -offset / totalLength)),
						(float)(CylinderTextureH2 + j * cornerArcLength / cornerArcSegmentCount * ratio)
					};
				}

				return sample;
			};

			auto MakeCornerPoint = [&](const SCornerSample& sample, const SimpleMath::Vector3& faceNormal) -> SGeometryPoint
			{
				SGeometryPoint node;
				node.vertice = sample.pos;
				node.normal = bNormalInward ? -faceNormal : faceNormal;
				node.ptOutView = CPointD(sample.uv.x, sample.uv.y);
				node.color = color;
				return node;
			};

			for (int j = 0; j < cornerArcSegmentCount; ++j)
			{
				SCornerSample samples[4] =
				{
					SampleCorner(0, j),
					SampleCorner(0, j + 1),
					SampleCorner(1, j + 1),
					SampleCorner(1, j),
				};

				SimpleMath::Vector3 edge0 = samples[1].pos - samples[0].pos;
				SimpleMath::Vector3 edge1 = samples[3].pos - samples[0].pos;
				SimpleMath::Vector3 faceNormal = edge0.Cross(edge1);
				if (faceNormal.LengthSquared() < 1e-12)
					continue;

				faceNormal.Normalize();

				SimpleMath::Vector3 outwardRef = samples[0].pos - cornerVertexInfo[curr].internalCylinderCenter;
				if (outwardRef.LengthSquared() > 1e-12 && faceNormal.Dot(outwardRef) < 0.0)
					faceNormal = -faceNormal;

				SSegmentUnitData sPreUnitR;
				sPreUnitR.vecSegPoints.push_back(MakeCornerPoint(samples[0], faceNormal));
				sPreUnitR.vecSegPoints.push_back(MakeCornerPoint(samples[1], faceNormal));
				sPreUnitR.vecSegPoints.push_back(MakeCornerPoint(samples[2], faceNormal));
				sPreUnitR.vecSegPoints.push_back(MakeCornerPoint(samples[3], faceNormal));

				geoData.vecSegNodes.push_back(sPreUnitR);
			}
		};

		if (n < 3 || !bClose)
		{
			using namespace DirectX::SimpleMath;
			std::vector<Vector2> vecEdgeNormal(n, Vector2());			//所有边的法向
			std::vector<bool> vecEdgeInLeft(n, 0);					//所有边的法向的连续性

			// 初始化边界值
			double min_x = std::numeric_limits<double>::max();
			double max_x = std::numeric_limits<double>::lowest();
			double min_y = std::numeric_limits<double>::max();
			double max_y = std::numeric_limits<double>::lowest();

			// 遍历所有顶点，更新边界
			for (const auto& point : vecVertexs)
			{
				min_x = std::min(min_x, point.x);
				max_x = std::max(max_x, point.x);
				min_y = std::min(min_y, point.y);
				max_y = std::max(max_y, point.y);
			}

			// 计算包围盒中心
			CPointD center((min_x + max_x) / 2.0, (min_y + max_y) / 2.0);

			//初始化边法向

			int edgeCount = n - 1;
			if (bBottomfaceCorner || bUpperfaceCorner)
				edgeCount = n;

			for (int i = 0; i < edgeCount; ++i)
			{
				CPointD& p1 = vecVertexs[i];
				CPointD& p2 = vecVertexs[(i + 1) % n];

				// 左手法向量
				vecEdgeNormal[i] = Vector2(-p2.y + p1.y, p2.x - p1.x);

				Vector2 toCenter(center.x - p1.x, center.y - p1.y);

				if (vecEdgeNormal[i].Dot(toCenter) < 0)
				{
					vecEdgeNormal[i] = vecEdgeNormal[i] * (-1.0);
					vecEdgeInLeft[i] = false;
				}
				else
				{
					vecEdgeInLeft[i] = true;
				}
			}

			int countTrue = 0;
			int countFalse = 0;
			for (bool b : vecEdgeInLeft)
			{
				if (b) countTrue++;
				else  countFalse++;
			}
			bool minorityValue = (countTrue <= countFalse);

			for (int i = 0; i < edgeCount; ++i)
			{
				CPointD& p1 = vecVertexs[i];
				CPointD& p2 = vecVertexs[(i + 1) % n];

				if (vecEdgeInLeft[i] == minorityValue)
					vecEdgeNormal[i] = -vecEdgeNormal[i];

				vecEdgeNormal[i].Normalize();

				vertexInfos[i].edgeNormal = SimpleMath::Vector3(-vecEdgeNormal[i].x, 0, -vecEdgeNormal[i].y);
				vertexInfos[i].edgeTangent = XMFLOAT4(p2.x - p1.x, 0, p2.y - p1.y, 0.f);

				if (!bNormalInward)
					vecEdgeNormal[i] = -vecEdgeNormal[i];
			}

			if (bUpperfaceCorner)
			{
				CreateGeometryPt(true);
			}

			if (bBottomfaceCorner)
			{
				CreateGeometryPt(false);
			}

			if (bBottomFace || bUpperFace)
			{
				GenerateFaces();
			}

			for (int i = 0; i < n - 1; ++i)
			{
				if (!vecFacePresent[i])
					continue;
				InsertCylinderCommonData(vecVertexs[i], vecVertexs[i + 1], i,
					XMFLOAT3(vecEdgeNormal[i].x, 0, vecEdgeNormal[i].y),
					XMFLOAT4(vecEdgeNormal[i].y, 0, -vecEdgeNormal[i].x, 1.0f));
			}

			if (!bUpperfaceCorner && !bBottomfaceCorner)
				return geoData;

			for (int i = 0; i < n - 1; ++i)
			{
				int prev = (i - 1 + n) % n;
				int next = (i + 1) % n;
				int curr = i;

				if (!vecFacePresent[i])
					continue;

				if (bUpperfaceCorner)
					AddNewEdgeRCorner(true, curr, prev, next, cornerAngle);

				if (bBottomfaceCorner)
					AddNewEdgeRCorner(false, curr, prev, next, -cornerAngle);
			}
		}
		else
		{
			std::vector<int> polygon;
			polygon.resize(n);
			std::iota(polygon.begin(), polygon.end(), 0);

			int posNum = n;
			while (posNum >= 3)
			{
				int ear = -1;
				if (posNum == 3)
					ear = 1;
				else
					ear = EarClipping(polygon, vecVertexs, bClockwise);

				if (ear == -1)
					break;

				int prev = polygon[(ear - 1 + posNum) % posNum];
				int next = polygon[(ear + 1) % posNum];
				int curr = polygon[ear];

				CPointD& prevPos = vecVertexs[prev];
				CPointD& currPos = vecVertexs[curr];
				CPointD& nextPos = vecVertexs[next];

				//对找到的三角形生成柱面
				CPointD centroid;
				centroid.x = (prevPos.x + currPos.x + nextPos.x) / 3.0;
				centroid.y = (prevPos.y + currPos.y + nextPos.y) / 3.0;

				auto CalcNormal = [&](CPointD& beginPos, CPointD& endPos, CPointD& centerPos, int nBeginIndex)
				{
					// beginPos - endPos 向量
					double abx = endPos.x - beginPos.x;
					double aby = endPos.y - beginPos.y;

					//  centerPos 相对于beginPos 的坐标
					double acx = centerPos.x - beginPos.x;
					double acy = centerPos.y - beginPos.y;

					// 叉积判断centerPos位于向量的左边还是右边
					double cross = abx * acy - aby * acx;

					// 法向量候选
					double normalX, normalZ;

					// cross > 0				
					normalX = -aby;
					normalZ = abx;

					if (cross < 0)
					{
						normalX = aby;
						normalZ = -abx;
					}

					vertexInfos[nBeginIndex].edgeNormal = SimpleMath::Vector3(-normalX, 0, -normalZ);
					vertexInfos[nBeginIndex].edgeNormal.Normalize();

					vertexInfos[nBeginIndex].edgeTangent = XMFLOAT4(abx, 0, aby, 1.0f);
				};

				if (prev == ((curr - 1 + n) % n))
				{
					CalcNormal(prevPos, currPos, centroid, prev);
				}
				if (next == ((curr + 1) % n))
				{
					CalcNormal(currPos, nextPos, centroid, curr);
				}
				if (prev == ((next + 1) % n))
				{
					CalcNormal(nextPos, prevPos, centroid, next);
				}


				polygon.erase(polygon.begin() + ear);
				posNum--;
			}


			if (bUpperfaceCorner)
			{
				CreateGeometryPt(true);
			}

			if (bBottomfaceCorner)
			{
				CreateGeometryPt(false);
			}

			if (bBottomFace || bUpperFace)
			{
				GenerateFaces();
			}

			//生成柱面
			for (auto iter : vertexInfos)
			{
				XMFLOAT3 normal;
				if (!bNormalInward)
				{
					normal = XMFLOAT3(iter.second.edgeNormal.x, 0, iter.second.edgeNormal.z);
				}
				else
				{
					normal = XMFLOAT3(-iter.second.edgeNormal.x, 0, -iter.second.edgeNormal.z);
				}

				if (!vecFacePresent[iter.first])
					continue;

				InsertCylinderCommonData(vecVertexs[iter.first], vecVertexs[(iter.first + 1) % n], iter.first,
					normal, iter.second.edgeTangent);
			}

			if (!bUpperfaceCorner && !bBottomfaceCorner)
				return geoData;

			for (int i = 0; i < n; ++i)
			{
				int prev = (i - 1 + n) % n;
				int next = (i + 1) % n;
				int curr = i;

				if (!vecFacePresent[i])
					continue;

				if (bUpperfaceCorner)
					AddNewEdgeRCorner(true, curr, prev, next, cornerAngle);

				if (bBottomfaceCorner)
					AddNewEdgeRCorner(false, curr, prev, next, -cornerAngle);
			}
		}

		return geoData;
	}

	GeometryData CreateRotationSolid(const std::vector<std::vector<CPointD>>& vecPointVector, const std::vector<bool>& vecClosed,
		const int nSteps, const int nAngle, const bool bNormalInward, const int nSubdivisionAmount, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		if (vecPointVector.empty())
			return GeometryData();

		//轮廓数据分段，确保任意轮廓不存在跨越y轴的边
		int nPointVector = vecPointVector.size();				// 旋转轮廓数量	
		size_t nValidEdge = 0;									// 有效边的总数
		std::vector<std::vector<CPointD>> vecRealPointVector;
		std::vector<bool> vecRealClosed;
		for (int i = 0; i < nPointVector; i++)
		{
			auto& vecPoints = vecPointVector[i];
			int n = vecPoints.size();
			if (n < 2)
				continue;

			vecRealClosed.push_back(vecClosed[i]);

			vecRealPointVector.push_back(std::vector<CPointD>());
			auto& currVecRealPoints = vecRealPointVector.back();
			currVecRealPoints.push_back(vecPoints[0]);

			for (int j = 1; j < n; j++)
			{
				bool bBeginInY = std::abs(vecPoints[j - 1].x) < 1e-12;
				bool bEndInY = std::abs(vecPoints[j].x) < 1e-12;

				//跨越分成两段
				if (!bBeginInY && !bEndInY && vecPoints[j - 1].x * vecPoints[j].x < 0)
				{
					double t = (0 - vecPoints[j - 1].x) / (vecPoints[j].x - vecPoints[j - 1].x);
					double yIntersect = vecPoints[j - 1].y + t * (vecPoints[j].y - vecPoints[j - 1].y);
					currVecRealPoints.push_back(CPointD(0, yIntersect));
					nValidEdge++;
				}

				currVecRealPoints.push_back(vecPoints[j]);
				if (!bBeginInY || !bEndInY)	//线段在Y轴上不算有效边
					nValidEdge++;
			}

		}

		//初始化轮廓创建需要的数据
		nPointVector = vecRealPointVector.size();
		int nFaceSubdivideNum = std::max(1, static_cast<int>(std::round(static_cast<double>(nSubdivisionAmount) / nSteps)));//面的细分数量
		size_t vertexNum = 0;											// 当前已经添加的顶点数
		double dShapeTextrueH = 1.0f / nPointVector;					// 单个轮廓的纹理高度	
		int temp = nValidEdge * (nSteps + 1) * nFaceSubdivideNum * 6;
		bool bOverU16 = (nValidEdge * (nSteps + 1) * nFaceSubdivideNum * 6) > 65535;// 可计算的索引数是否超过了uint16_t（65535）		

		//旋转角度，弧度制
		double radianAngle = nAngle * PI / 180;

		//一个面的索引顺序、纹理坐标范围
		double arrTexcoord[4] = { 0, 0, 0, 0 };

		//开始创建数据
		GeometryData geoData;

		//对两点连线建立旋转曲面
		std::function<void(CPointD&, CPointD&, CPointD&)> AddRevolve = [&](CPointD& beginPos, CPointD& endPos, CPointD& normal2D)
		{
			bool bBeginInY = std::abs(beginPos.x) < 1e-12;
			bool bEndInY = std::abs(endPos.x) < 1e-12;

			//线段在Y轴上直接跳过
			if (bBeginInY && bEndInY)
				return;

			//初始顶点和法向
			XMFLOAT3 P1(beginPos.x, beginPos.y, 0);
			XMFLOAT3 P2(endPos.x, endPos.y, 0);
			XMFLOAT3 normals(normal2D.x, normal2D.y, 0);

			for (int j = 1; j <= (nSteps + 1); j++)
			{
				XMFLOAT3 nextP1 = P1;
				XMFLOAT3 nextP2 = P2;

				//计算点绕Y轴旋后的坐标
				double angleRadian = radianAngle * j / (nSteps + 1);
				double cosA = std::cos(angleRadian);
				double sinA = std::sin(angleRadian);

				if (!bBeginInY)
					nextP1 = XMFLOAT3(beginPos.x * cosA, beginPos.y, beginPos.x * sinA);

				if (!bEndInY)
					nextP2 = XMFLOAT3(endPos.x * cosA, endPos.y, endPos.x * sinA);

				// 平面上的两个向量
				XMFLOAT3 P3 = bBeginInY ? nextP2 : nextP1;
				XMVECTOR v1 = XMVectorSet(P2.x - P1.x, P2.y - P1.y, P2.z - P1.z, 0.0f);
				XMVECTOR v2 = XMVectorSet(P3.x - P1.x, P3.y - P1.y, P3.z - P1.z, 0.0f);

				// 法向量 = v1 × v2
				XMVECTOR normalized = XMVector3Normalize(XMVector3Cross(v1, v2));

				XMFLOAT3 PlaneNormal;
				XMStoreFloat3(&PlaneNormal, normalized);

				// 判断方向是否一致
				float d = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&normals), normalized));
				if (d < 0.0f) {
					PlaneNormal.x = -PlaneNormal.x;
					PlaneNormal.y = -PlaneNormal.y;
					PlaneNormal.z = -PlaneNormal.z;
				}

				//法向方向矫正	
				normals = PlaneNormal;

				//计算切向量
				XMFLOAT3 tangent;
				XMStoreFloat3(&tangent, XMVector3Normalize(XMVectorSet(P2.x - P1.x, P2.y - P1.y, P2.z - P1.z, 0.0f)));

				//x轴纹理偏移
				arrTexcoord[0] = (double)(j - 1) / (nSteps + 1);
				arrTexcoord[2] = (double)j / (nSteps + 1);

				//插入顶点
				XMFLOAT3 IncrementP1((nextP1.x - P1.x) / nFaceSubdivideNum, 0, (nextP1.z - P1.z) / nFaceSubdivideNum);
				XMFLOAT3 IncrementP2((nextP2.x - P2.x) / nFaceSubdivideNum, 0, (nextP2.z - P2.z) / nFaceSubdivideNum);

				double IncrementTexcoord = (arrTexcoord[2] - arrTexcoord[0]) / nFaceSubdivideNum;
				for (int i = 1; i <= nFaceSubdivideNum; i++)
				{
					XMFLOAT3 SubP1(IncrementP1.x + P1.x, IncrementP1.y + P1.y, IncrementP1.z + P1.z);
					XMFLOAT3 SubP2(IncrementP2.x + P2.x, IncrementP2.y + P2.y, IncrementP2.z + P2.z);

					// 插入 UV
					arrTexcoord[2] = arrTexcoord[0] + IncrementTexcoord;

					auto AddPoint = [&](XMFLOAT3 vertice, XMFLOAT2 samplePos, bool bPre)->SGeometryPoint {

						SGeometryPoint node;

						node.vertice = vertice;
						node.ptOutView = CPointD(samplePos.x, samplePos.y);
						node.normal = normals;
						node.tangent = XMFLOAT4(tangent.x, tangent.y, tangent.z, 1.0f);
						node.color = color;


						return node;
					};

					SSegmentUnitData sPreNode;

					sPreNode.vecSegPoints.push_back(AddPoint(P1, XMFLOAT2(arrTexcoord[0], arrTexcoord[1]), false));
					sPreNode.vecSegPoints.push_back(AddPoint(SubP1, XMFLOAT2(arrTexcoord[2], arrTexcoord[1]), false));
					sPreNode.vecSegPoints.push_back(AddPoint(SubP2, XMFLOAT2(arrTexcoord[2], arrTexcoord[3]), false));
					sPreNode.vecSegPoints.push_back(AddPoint(P2, XMFLOAT2(arrTexcoord[0], arrTexcoord[3]), false));

					geoData.vecSegNodes.push_back(sPreNode);

					arrTexcoord[0] = arrTexcoord[2];
					P1 = SubP1;
					P2 = SubP2;
				}

				P1 = nextP1;
				P2 = nextP2;
			}
		};

		for (int i = 0; i < nPointVector; i++)
		{
			int n = vecRealPointVector[i].size();
			if (n < 2)
				continue;

			bool bClose = vecRealClosed[i];

			std::vector<CPointD> vecVertexs;
			vecVertexs.resize(n);

			std::vector<double> vecVertexTextrueH;
			vecVertexTextrueH.resize(bClose ? n + 1 : n);

			double sum = 0.0;
			double totalLength = 0.0;
			vecVertexs[0].x = vecRealPointVector[i][0].x / 1000.0f;
			vecVertexs[0].y = vecRealPointVector[i][0].y / 1000.0f;
			vecVertexTextrueH[0] = 0;

			for (int j = 1; j < n; ++j)
			{
				auto& p = vecRealPointVector[i][j];

				vecVertexs[j].x = p.x / 1000.0f;
				vecVertexs[j].y = p.y / 1000.0f;

				totalLength += sqrt(pow(vecVertexs[j].x - vecVertexs[j - 1].x, 2) + pow(vecVertexs[j].y - vecVertexs[j - 1].y, 2));
				vecVertexTextrueH[j] = totalLength;

				if (bClose)
				{
					auto& pNext = vecRealPointVector[i][(j + 1) % n];
					sum += (pNext.x - p.x) * (pNext.y + p.y);  // 等价于 2 * area
				}
			}
			sum += (vecRealPointVector[i][1].x - vecRealPointVector[i][0].x) * (vecRealPointVector[i][1].y + vecRealPointVector[i][0].y);

			//计算y轴坐标
			int TextrueNum = n;
			if (bClose)
			{
				TextrueNum++;
				totalLength += sqrt(pow(vecVertexs[0].x - vecVertexs[n - 1].x, 2) + pow(vecVertexs[0].y - vecVertexs[n - 1].y, 2));
				vecVertexTextrueH[n] = totalLength;
			}
			double ratio = dShapeTextrueH / totalLength;
			double dTextrueHBegin = dShapeTextrueH * i;
			for (int j = 0; j < TextrueNum; j++)
			{
				vecVertexTextrueH[j] = dTextrueHBegin + vecVertexTextrueH[j] * ratio;
			}

			bool bClockwise = sum > 0;	//底面形状的点序是顺时针还是逆时针

			if (n < 3 || !bClose)
			{
				using namespace DirectX::SimpleMath;

				std::vector<Vector2> vecEdgeNormal(n, Vector2());			//所有边的法向
				std::deque<int> queHorizontalEdge;							//水平边的序号
				size_t nInvalidEdge = 0;									//无效边（在y轴上）的数量

				std::vector<int> vecEdgeCoherence(n, 0);					//所有边的法向的连续性
				int initCoherence = 1;										//初始边的连续性
				double lowY = vecVertexs[0].y, highY = vecVertexs[0].y;		//边的重叠判断辅助变量

				bool bAllEdgeInleft = vecVertexs[0].x < 0 || vecVertexs[1].x < 0;		//所有边的位置是否连续在一边
				int nFirstIndex = 0;													//连续在一边的边的起点
				//初始化所有非水平的边
				for (int i = 0; i < n - 1; ++i)
				{
					//边的向量
					Vector2 EdgeVec(vecVertexs[i].x - vecVertexs[i + 1].x, vecVertexs[i].y - vecVertexs[i + 1].y);

					// 如果 EdgeVec 是零向量（两点重合）或者在y轴上
					if (EdgeVec.LengthSquared() < 1e-8f || (std::abs(vecVertexs[i].x) < 1e-8f && std::abs(vecVertexs[i + 1].x) < 1e-8f))
					{
						nInvalidEdge++;
						continue;
					}

					//新的边位置到另一侧了，需要对前一段连续在一侧的边进行处理
					bool bEdgeInleft = vecVertexs[i + 1].x < 0 || vecVertexs[i].x < 0;

					//水平向量无法直接判断
					if (fabs(EdgeVec.y) < 1e-8)
					{
						queHorizontalEdge.push_back(i);
						continue;
					}

					// 归一化
					EdgeVec.Normalize();

					//假定y轴的方向向量为（0，1）,确定向内方向：
					//当边在y轴左边时，
					//如果边的方向向量在2、3象限，边的方向向量和y轴向量的点积符号和应该向内的垂直向量和y轴向量的点积符号相同；
					//如果边的方向向量在1、4象限，边的方向向量和y轴向量的点积符号和应该向内的垂直向量和y轴向量的点积符号相反；
					//如果边的方向向量和y轴的方向向量平行，则向内的垂直向量固定为（1，0）
					//当边在y轴右边时，
					//如果边的方向向量在2、3象限，边的方向向量和y轴向量的点积符号和应该和向内的垂直向量和y轴向量的点积符号相反；
					//如果边的方向向量在1、4象限，边的方向向量和y轴向量的点积符号和应该和向内的垂直向量和y轴向量的点积符号相同；
					//如果边的方向向量和y轴的方向向量平行，则向内的垂直向量固定为（-1，0）
					if (fabs(EdgeVec.x) < 1e-8)
					{
						if (bEdgeInleft)
							vecEdgeNormal[i] = Vector2(1.0f, 0.0f);
						else
							vecEdgeNormal[i] = Vector2(-1.0f, 0.0f);
					}
					else
					{
						//垂直向量两个备选
						Vector2 VerticalVec1(EdgeVec.y, -EdgeVec.x);
						Vector2 VerticalVec2(-EdgeVec.y, EdgeVec.x);
						VerticalVec1.Normalize();
						VerticalVec2.Normalize();

						// 使用点积判断方向
						bool useFirst = (VerticalVec1.Dot(Vector2(0, 1)) * EdgeVec.y) > 0;

						if ((bEdgeInleft && EdgeVec.x < 0) || (!bEdgeInleft && EdgeVec.x >= 0))
							vecEdgeNormal[i] = useFirst ? VerticalVec1 : VerticalVec2;
						else
							vecEdgeNormal[i] = useFirst ? VerticalVec2 : VerticalVec1;
					}

					//根据向内标志选择向内向外
					if (!bNormalInward)
						vecEdgeNormal[i] *= -1.0f;

					// 判断是否有交集
					if (bEdgeInleft != bAllEdgeInleft)
					{
						lowY = std::min(vecVertexs[i].y, vecVertexs[i + 1].y);
						highY = std::max(vecVertexs[i].y, vecVertexs[i + 1].y);
					}
					else
					{
						double a = std::min(vecVertexs[i].y, vecVertexs[i + 1].y);
						double b = std::max(vecVertexs[i].y, vecVertexs[i + 1].y);
						if (a < highY && b > lowY)
						{
							lowY = a;
							highY = b;
							initCoherence *= -1;
						}
						else
						{
							lowY = std::min(lowY, a);
							highY = std::max(highY, b);
						}
					}
					vecEdgeCoherence[i] = initCoherence;

					//更新连续性
					if (bEdgeInleft != bAllEdgeInleft || i == n - 2)
					{
						int last = i == n - 2 ? i + 1 : i;
						int Coherence = 0;
						for (int j = nFirstIndex; j < last; j++)
						{
							double tempX = (vecVertexs[j].x - vecVertexs[j + 1].x);
							double tempY = (vecVertexs[j].y - vecVertexs[j + 1].y);
							Coherence += vecEdgeCoherence[j] * (tempX*tempX + tempY * tempY);
						}
						if (Coherence >= 0)
							Coherence = 1;
						else
							Coherence = -1;

						for (int j = nFirstIndex; j < last; j++)
						{
							if (vecEdgeCoherence[j] != Coherence && vecEdgeCoherence[j] != 0)
							{
								vecEdgeNormal[j] *= -1.0f;
								vecEdgeCoherence[j] = 2;
							}
						}

						nFirstIndex = i;
						bAllEdgeInleft = bEdgeInleft;
					}

				}

				//处理水平的边
				if ((queHorizontalEdge.size() + nInvalidEdge) != n)	//并非所有边都是水平边
				{
					while (!queHorizontalEdge.empty())
					{
						int nCurrIndex = queHorizontalEdge.front();
						queHorizontalEdge.pop_front();

						int nPrevIndex = nCurrIndex - 1;
						while (nPrevIndex >= 0)
						{
							if (vecEdgeNormal[nPrevIndex].x != 0 || vecEdgeNormal[nPrevIndex].y != 0)
								break;
							else
								nPrevIndex--;
						}

						int nNextIndex = nCurrIndex + 1;
						while (nNextIndex < n)
						{
							if (vecEdgeNormal[nNextIndex].x != 0 || vecEdgeNormal[nNextIndex].y != 0)
								break;
							else
								nNextIndex++;
						}

						int nCloestNormal = -1;
						if (nPrevIndex >= 0 || nNextIndex < n)
						{
							if (nPrevIndex < 0)					//如果其中一侧不存在（如越界），就选另一侧
								nCloestNormal = nNextIndex;
							else if (nNextIndex >= n)
								nCloestNormal = nPrevIndex;
							else
							{
								bool bPrevInLeft = vecVertexs[nPrevIndex].x + vecVertexs[nPrevIndex + 1].x < 0;
								bool bCurrInLeft = vecVertexs[nCurrIndex].x + vecVertexs[nCurrIndex + 1].x < 0;
								bool bNextInLeft = vecVertexs[nNextIndex].x + vecVertexs[nNextIndex + 1].x < 0;

								if (bCurrInLeft == bPrevInLeft && bCurrInLeft == bNextInLeft) //判断当前边与前/后边是否在同一侧
								{
									//根据距离远近决定
									if (nCurrIndex - nPointVector > nNextIndex - nCurrIndex)
										nCloestNormal = nPrevIndex;
									else
										nCloestNormal = nNextIndex;
								}
								else if (bCurrInLeft == bPrevInLeft) //若仅与某一侧一致，则选那一侧
									nCloestNormal = nPrevIndex;
								else if (bCurrInLeft == bNextInLeft)
									nCloestNormal = nNextIndex;
							}
						}

						if (nCloestNormal != -1)
						{
							if (vecEdgeCoherence[nCloestNormal] == 0)
								vecEdgeNormal[nCurrIndex].y = vecEdgeNormal[nCloestNormal].y;		//最靠近的是水平边
							else
							{
								bool bEdgeInleft = vecVertexs[nCloestNormal + 1].x < 0 || vecVertexs[nCloestNormal].x < 0;
								//查找更靠近水平轴的那个点来判断方向向量的y,最靠近边相对水平边是更靠近y轴还是远离
								double yComponent = 0;
								bool nearY = false;
								if (fabs(vecVertexs[nCloestNormal].y - vecVertexs[nCurrIndex].y) > fabs(vecVertexs[nCloestNormal + 1].y - vecVertexs[nCurrIndex].y))
								{
									yComponent = vecVertexs[nCloestNormal].y - vecVertexs[nCloestNormal + 1].y;
									if (bEdgeInleft)
										nearY = vecVertexs[nCloestNormal + 1].x >= vecVertexs[nCurrIndex].x&&vecVertexs[nCloestNormal + 1].x >= vecVertexs[nCurrIndex + 1].x;
									else
										nearY = vecVertexs[nCloestNormal + 1].x <= vecVertexs[nCurrIndex].x&&vecVertexs[nCloestNormal + 1].x <= vecVertexs[nCurrIndex + 1].x;
								}
								else
								{
									yComponent = vecVertexs[nCloestNormal + 1].y - vecVertexs[nCloestNormal].y;
									if (bEdgeInleft)
										nearY = vecVertexs[nCloestNormal].x >= vecVertexs[nCurrIndex].x&&vecVertexs[nCloestNormal].x >= vecVertexs[nCurrIndex + 1].x;
									else
										nearY = vecVertexs[nCloestNormal].x <= vecVertexs[nCurrIndex].x&&vecVertexs[nCloestNormal].x <= vecVertexs[nCurrIndex + 1].x;

								}

								if ((yComponent < 0) == nearY)
								{
									if (bEdgeInleft)
									{
										if (vecEdgeNormal[nCloestNormal].x > 0)
											vecEdgeNormal[nCurrIndex].y = 1;
										else
											vecEdgeNormal[nCurrIndex].y = -1;
									}
									else
									{
										if (vecEdgeNormal[nCloestNormal].x > 0)
											vecEdgeNormal[nCurrIndex].y = -1;
										else
											vecEdgeNormal[nCurrIndex].y = 1;
									}
								}
								else
								{
									if (bEdgeInleft)
									{
										if (vecEdgeNormal[nCloestNormal].x > 0)
											vecEdgeNormal[nCurrIndex].y = -1;
										else
											vecEdgeNormal[nCurrIndex].y = 1;
									}
									else
									{
										if (vecEdgeNormal[nCloestNormal].x > 0)
											vecEdgeNormal[nCurrIndex].y = 1;
										else
											vecEdgeNormal[nCurrIndex].y = -1;
									}
								}
								/*
																if(vecEdgeCoherence[nCloestNormal] == 2)
																	vecEdgeNormal[nCurrIndex].y *= -1;*/
							}
						}
					}
				}

				for (int i = 0; i < n - 1; ++i)
				{
					arrTexcoord[1] = vecVertexTextrueH[i];
					arrTexcoord[3] = vecVertexTextrueH[i + 1];

					AddRevolve(vecVertexs[i], vecVertexs[i + 1], CPointD(vecEdgeNormal[i].x, vecEdgeNormal[i].y));
				}
			}
			else
			{
				int posNum = n;

				std::vector<int> polygon;
				polygon.resize(n);
				std::iota(polygon.begin(), polygon.end(), 0);

				while (posNum >= 3)
				{
					int ear = -1;
					if (posNum == 3)
						ear = 1;
					else
						ear = EarClipping(polygon, vecVertexs, bClockwise);

					if (ear == -1)
						break;

					int prev = polygon[(ear - 1 + posNum) % posNum];
					int next = polygon[(ear + 1) % posNum];
					int curr = polygon[ear];

					CPointD& prevPos = vecVertexs[prev];
					CPointD& currPos = vecVertexs[curr];
					CPointD& nextPos = vecVertexs[next];

					//对找到的三角形生成						
					CPointD centroid;
					centroid.x = (prevPos.x + currPos.x + nextPos.x) / 3.0;
					centroid.y = (prevPos.y + currPos.y + nextPos.y) / 3.0;

					auto CalEdgeToRevolve = [&](CPointD& beginPos, CPointD& endPos, CPointD& centerPos, int beginIndex)->void
					{
						// beginPos - endPos 向量
						double abx = endPos.x - beginPos.x;
						double aby = endPos.y - beginPos.y;

						//  centerPos 相对于beginPos 的坐标
						double acx = centerPos.x - beginPos.x;
						double acy = centerPos.y - beginPos.y;

						// 叉积判断centerPos位于向量的左边还是右边
						double cross = abx * acy - aby * acx;

						// 法向量候选
						double normalX = -aby, normalY = abx;

						if (cross < 0)
						{
							normalX = aby;
							normalY = -abx;
						}

						if (!bNormalInward)
						{
							normalX = -normalX;
							normalY = -normalY;
						}

						// 归一化
						double length = std::sqrt(normalX * normalX + normalY * normalY);
						if (length > 1e-8)
						{
							normalX /= length;
							normalY /= length;
						}


						arrTexcoord[1] = vecVertexTextrueH[beginIndex];
						arrTexcoord[3] = vecVertexTextrueH[beginIndex + 1];
						AddRevolve(beginPos, endPos, CPointD(normalX, normalY));

					};

					if (prev == ((curr - 1 + n) % n))
						CalEdgeToRevolve(prevPos, currPos, centroid, prev);
					if (next == ((curr + 1) % n))
						CalEdgeToRevolve(currPos, nextPos, centroid, curr);
					if (prev == ((next + 1) % n))
						CalEdgeToRevolve(nextPos, prevPos, centroid, next);

					polygon.erase(polygon.begin() + ear);
					posNum--;
				}
			}
		}

		geoData.UpdateverticeDatas();

		//if(geoData.vertices.empty())
		//	return GeometryData();

		//DirectX::BoundingBox::CreateFromPoints(geoData.boundingbox, geoData.vertices.size(),
		//	(const DirectX::XMFLOAT3*)geoData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return geoData;
	}

}
