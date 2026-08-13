#pragma once
#include "RenderCommonGlobal.h"
#include "Common/Common/CommonRapidJson.h"
#include "Common/Common/Maths/Point.h"
#include "Common/Common/Maths/Line.h"
#include "Common/Common/CommonDefine.h"

#include <map>
#include <array>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <functional>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <boost/geometry.hpp>

#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


#define GeomID_Xml          QString("GeomID")
#define GeomVertice_Xml     QString("GeomVertice")
#define GeomNormal_Xml      QString("GeomNormal")
#define GeomTexInput_Xml   QString("GeomTexInput")
#define GeomTexOutput_Xml   QString("GeomTexOutput")
#define GeomTangent_Xml     QString("GeomTangent")
#define GeomBitangent_Xml   QString("GeomBitangent")
#define GeomColor_Xml       QString("GeomColor")


#define SlicePoints  "SlicePoints"
#define Data_InputArr_Xml    "Pt"

#define Json_GeomData QString("GeometryData")


static bool LoadFloat2RapidJson(DirectX::XMFLOAT2& float2, const rapidjson::Value& arr)
{
	if (!arr.IsArray() || arr.Size() != 2)
		return false;

	double x = 0.0;
	double y = 0.0;
	if (!RapidJsonValueToDouble(arr[0], x) || !RapidJsonValueToDouble(arr[1], y))
		return false;

	float2.x = static_cast<float>(x);
	float2.y = static_cast<float>(y);
	return true;
}

static bool LoadCPointDRapidJson(CPointD& pt, const rapidjson::Value& arr)
{
	if (!arr.IsArray() || arr.Size() != 2)
		return false;

	if (!RapidJsonValueToDouble(arr[0], pt.x) || !RapidJsonValueToDouble(arr[1], pt.y))
		return false;

	return true;
}

static bool LoadFloat3RapidJson(DirectX::XMFLOAT3& float3, const rapidjson::Value& arr)
{
	if (!arr.IsArray() || arr.Size() != 3)
		return false;

	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	if (!RapidJsonValueToDouble(arr[0], x)
		|| !RapidJsonValueToDouble(arr[1], y)
		|| !RapidJsonValueToDouble(arr[2], z))
		return false;

	float3.x = static_cast<float>(x);
	float3.y = static_cast<float>(y);
	float3.z = static_cast<float>(z);
	return true;
}

static bool LoadFloat4RapidJson(DirectX::XMFLOAT4& float4, const rapidjson::Value& arr)
{
	if (!arr.IsArray() || arr.Size() != 4)
		return false;

	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	double w = 0.0;
	if (!RapidJsonValueToDouble(arr[0], x)
		|| !RapidJsonValueToDouble(arr[1], y)
		|| !RapidJsonValueToDouble(arr[2], z)
		|| !RapidJsonValueToDouble(arr[3], w))
		return false;

	float4.x = static_cast<float>(x);
	float4.y = static_cast<float>(y);
	float4.z = static_cast<float>(z);
	float4.w = static_cast<float>(w);
	return true;
}

static rapidjson::Value SaveFloat2RapidJson(const DirectX::XMFLOAT2& float2, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arr(rapidjson::kArrayType);
	arr.PushBack(RapidJsonSafeFloat(float2.x), allocator);
	arr.PushBack(RapidJsonSafeFloat(float2.y), allocator);
	return arr;
}

static rapidjson::Value SaveCPointDRapidJson(const CPointD& pt, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arr(rapidjson::kArrayType);
	arr.PushBack(RapidJsonSafeDouble(pt.x), allocator);
	arr.PushBack(RapidJsonSafeDouble(pt.y), allocator);
	return arr;
}

static rapidjson::Value SaveFloat3RapidJson(const DirectX::XMFLOAT3& float3, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arr(rapidjson::kArrayType);
	arr.PushBack(RapidJsonSafeFloat(float3.x), allocator);
	arr.PushBack(RapidJsonSafeFloat(float3.y), allocator);
	arr.PushBack(RapidJsonSafeFloat(float3.z), allocator);
	return arr;
}

static rapidjson::Value SaveFloat4RapidJson(const DirectX::XMFLOAT4& float4, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arr(rapidjson::kArrayType);
	arr.PushBack(RapidJsonSafeFloat(float4.x), allocator);
	arr.PushBack(RapidJsonSafeFloat(float4.y), allocator);
	arr.PushBack(RapidJsonSafeFloat(float4.z), allocator);
	arr.PushBack(RapidJsonSafeFloat(float4.w), allocator);
	return arr;
}

struct SGeometryPoint
{
	DirectX::XMFLOAT3 vertice;	// 3D模型点
	DirectX::XMFLOAT3 normal;	// 法向量

	CPointD ptInView;	// 输入视图窗口采样位置
	CPointD ptOutView;	// 输出视图窗口采样位置

	DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f);	// 颜色

	DirectX::XMFLOAT4 tangent;		//切线， 暂时没用
	DirectX::XMFLOAT4 bitangent;	//双切线， 暂时没用

	SGeometryPoint()
	{

	}

	SGeometryPoint(const DirectX::XMFLOAT3 &ver,const DirectX::XMFLOAT3 &nor, const CPointD &texSample,const DirectX::XMFLOAT4 &clr)
		: vertice(ver)
		, normal(nor)
		, ptOutView(texSample)
		, color(clr)

	{
		
	}

	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		rapidjson::Value dataArr(rapidjson::kArrayType);

		dataArr.PushBack(SaveFloat3RapidJson(vertice, allocator), allocator);
		dataArr.PushBack(SaveFloat3RapidJson(normal, allocator), allocator);
		dataArr.PushBack(SaveCPointDRapidJson(ptInView, allocator), allocator);
		dataArr.PushBack(SaveCPointDRapidJson(ptOutView, allocator), allocator);
		dataArr.PushBack(SaveFloat4RapidJson(tangent, allocator), allocator);
		dataArr.PushBack(SaveFloat4RapidJson(bitangent, allocator), allocator);
		dataArr.PushBack(SaveFloat4RapidJson(color, allocator), allocator);

		rapidjson::Value key(Data_InputArr_Xml, allocator);
		root.AddMember(key, dataArr, allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& object)
	{
		if (!object.IsObject())
		{
			return false;
		}

		const char* key = Data_InputArr_Xml;
		if (!object.HasMember(key) || !object[key].IsArray())
		{
			return false;
		}

		const rapidjson::Value& dataArr = object[key];
		if (dataArr.Size() != 7)
		{
			return false;
		}

		if (!LoadFloat3RapidJson(vertice, dataArr[0]))
			return false;
		if (!LoadFloat3RapidJson(normal, dataArr[1]))
			return false;
		if (!LoadCPointDRapidJson(ptInView, dataArr[2]))
			return false;
		if (!LoadCPointDRapidJson(ptOutView, dataArr[3]))
			return false;
		if (!LoadFloat4RapidJson(tangent, dataArr[4]))
			return false;
		if (!LoadFloat4RapidJson(bitangent, dataArr[5]))
			return false;
		if (!LoadFloat4RapidJson(color, dataArr[6]))
			return false;

		return true;
	}
};

struct SSegmentUnitData
{
	std::vector<SGeometryPoint> vecSegPoints;	// 实际切片数据
	bool bConvex = true;

	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		rapidjson::Value segPointArr(rapidjson::kArrayType);
		for (const auto & segPoint : vecSegPoints)
		{
			rapidjson::Value pointValue(rapidjson::kObjectType);
			segPoint.SaveDataToRapidJson(pointValue, allocator);
			segPointArr.PushBack(pointValue, allocator);
		}
		root.AddMember(rapidjson::StringRef(SlicePoints), segPointArr, allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& object)
	{
		if (!object.IsObject() || !object.HasMember(SlicePoints) || !object[SlicePoints].IsArray())
		{
			return false;
		}

		const rapidjson::Value& segPointArr = object[SlicePoints];
		vecSegPoints.clear();
		for (rapidjson::SizeType i = 0; i < segPointArr.Size(); ++i)
		{
			vecSegPoints.push_back(SGeometryPoint());
			if (!vecSegPoints.back().LoadDataFromRapidJson(segPointArr[i]))
			{
				vecSegPoints.clear();
				return false;
			}
		}
		return true;
	}
};

//判断是否是 3D / 2D 同时近似重合的裁剪冗余点
static bool IsNearDuplicatePoint(const SGeometryPoint& a, const SGeometryPoint& b)
{
	const float vEps = 1e-5f;
	const double uvEps = 1e-5;

	const bool same3D =
		std::fabs(a.vertice.x - b.vertice.x) <= vEps &&
		std::fabs(a.vertice.y - b.vertice.y) <= vEps &&
		std::fabs(a.vertice.z - b.vertice.z) <= vEps;
	if (!same3D)
		return false;

	const bool same2D =
		std::fabs(a.ptOutView.x - b.ptOutView.x) <= uvEps &&
		std::fabs(a.ptOutView.y - b.ptOutView.y) <= uvEps;
	return same2D;
}

// 检查点是否在三角形内部
static bool isPointInTriangle2D(const SGeometryPoint& p, const SGeometryPoint& a, const SGeometryPoint& b, const SGeometryPoint& c)
{
	// 计算面积
	auto area = [](const SGeometryPoint& a, const SGeometryPoint& b, const SGeometryPoint& c) {
		return 0.5 * abs((b.ptOutView.x - a.ptOutView.x) *
			(c.ptOutView.y - a.ptOutView.y) -
			(c.ptOutView.x - a.ptOutView.x) * (b.ptOutView.y - a.ptOutView.y));
	};

	double total_area = area(a, b, c);
	double area1 = area(p, b, c);
	double area2 = area(p, a, c);
	double area3 = area(p, a, b);

	// 如果点在三角形内部，三个小三角形面积之和应等于大三角形面积
	return abs(total_area - (area1 + area2 + area3)) < 1e-6;
}

// 检查三点是否构成耳（Ear）
static bool isEar2D(const std::vector<SGeometryPoint>& poly, int prev, int curr, int next, std::vector<int> indices) {
	// 1. 检查内角是否为凸角（< 180度）- 核心修复逻辑
	const CPointD& p_prev = poly[prev].ptOutView;
	const CPointD& p_curr = poly[curr].ptOutView;
	const CPointD& p_next = poly[next].ptOutView;

	// 内联计算叉积：(p_curr - p_prev) × (p_next - p_curr)
	double v1x = p_curr.x - p_prev.x;
	double v1y = p_curr.y - p_prev.y;
	double v2x = p_next.x - p_curr.x;
	double v2y = p_next.y - p_curr.y;
	double cross = v1x * v2y - v1y * v2x;

	// 假设多边形顶点为逆时针排列：叉积>0是凸角，<=0（凹角/共线）则不是耳
	// 加入1e-8浮点误差容忍，避免精度问题
	if (cross <= 1e-8) {
		return false;
	}

	// 2. 检查三角形(prev, curr, next)是否包含其他顶点（内联点在三角形内的判断）
	for (int i = 0; i < indices.size(); i++)
	{
		int index = indices[i];

		if (index == prev || index == curr || index == next) continue;

		const CPointD& p = poly[index].ptOutView;

		// 内联计算点p与三角形三边的叉积，判断是否在内部
		// 计算 (b-a) × (p-a)
		double c1 = (p_curr.x - p_prev.x) * (p.y - p_prev.y) - (p_curr.y - p_prev.y) * (p.x - p_prev.x);
		// 计算 (c-b) × (p-b)
		double c2 = (p_next.x - p_curr.x) * (p.y - p_curr.y) - (p_next.y - p_curr.y) * (p.x - p_curr.x);
		// 计算 (a-c) × (p-c)
		double c3 = (p_prev.x - p_next.x) * (p.y - p_next.y) - (p_prev.y - p_next.y) * (p.x - p_next.x);

		// 判断是否全部同号（内部）
		bool hasNeg = (c1 < 0) || (c2 < 0) || (c3 < 0);
		bool hasPos = (c1 > 0) || (c2 > 0) || (c3 > 0);
		bool isInside = !(hasNeg && hasPos);

		// 如果有任何点在三角形内部，不是耳
		if (isInside) {
			return false;
		}
	}

	return true;
}

// 使有序多边形变成顺时针或逆时针排序
static void makePolyCounterClockwise(std::vector<SGeometryPoint>& poly, bool bCounterClockwise)
{
	if (poly.size() < 3)
		return;

	const double EPS = 1e-6;

	double area = 0.0;
	for (size_t i = 0; i < poly.size(); i++) {
		const auto& p1 = poly[i];
		const auto& p2 = poly[(i + 1) % poly.size()];
		area += (p1.ptOutView.x * p2.ptOutView.y - p2.ptOutView.x * p1.ptOutView.y);
	}
	if (bCounterClockwise)
	{
		if (area < -EPS) {
			std::reverse(poly.begin(), poly.end());
		}
	}
	else
	{
		if (area > EPS) {
			std::reverse(poly.begin(), poly.end());
		}
	}
}

// 二维三角剖分（Ear Clipping算法，处理凹多边形），要求多边形顶点为逆时针排列,坐标系x轴正方向向右，y轴正方向向上
// 返回三角形顶点在 polyPts 中的原始下标（不因调整绕序而改写点数组）
static std::vector<std::array<int, 3>> triangulatePolygon2DIndices(const std::vector<SGeometryPoint>& polyPts)
{
	if (polyPts.size() < 3) {
		return {};
	}

	const int n = static_cast<int>(polyPts.size());
	std::vector<int> indices(static_cast<size_t>(n));
	std::iota(indices.begin(), indices.end(), 0);

	// 仅反转索引环以保证逆时针，保留原始下标语义
	double area = 0.0;
	for (int i = 0; i < n; ++i) {
		const auto& p1 = polyPts[static_cast<size_t>(i)].ptOutView;
		const auto& p2 = polyPts[static_cast<size_t>((i + 1) % n)].ptOutView;
		area += (p1.x * p2.y - p2.x * p1.y);
	}
	if (area < -1e-6) {
		std::reverse(indices.begin(), indices.end());
	}

	std::vector<std::array<int, 3>> triangles;
	while (indices.size() > 3)
	{
		bool found_ear = false;
		for (int i = 0; i < static_cast<int>(indices.size()); i++) {
			int prev = indices[static_cast<size_t>((i - 1 + static_cast<int>(indices.size())) % static_cast<int>(indices.size()))];
			int curr = indices[static_cast<size_t>(i)];
			int next = indices[static_cast<size_t>((i + 1) % static_cast<int>(indices.size()))];

			if (isEar2D(polyPts, prev, curr, next, indices))
			{
				triangles.push_back({ prev, curr, next });
				indices.erase(indices.begin() + i);
				found_ear = true;
				break;
			}
		}

		if (!found_ear) {
			indices.pop_back();
		}
	}

	if (indices.size() == 3) {
		triangles.push_back({ indices[0], indices[1], indices[2] });
	}

	return triangles;
}

static std::vector<std::vector<SGeometryPoint>> triangulatePolygon2D(const std::vector<SGeometryPoint>& polyPts)
{
	const auto indexTris = triangulatePolygon2DIndices(polyPts);
	std::vector<std::vector<SGeometryPoint>> triangles;
	triangles.reserve(indexTris.size());
	for (const auto& t : indexTris) {
		triangles.push_back({ polyPts[static_cast<size_t>(t[0])],
			polyPts[static_cast<size_t>(t[1])],
			polyPts[static_cast<size_t>(t[2])] });
	}
	return triangles;
}

static bool TransPoly(const std::vector<SGeometryPoint>& vecSegPoints, std::vector<std::vector<SGeometryPoint>> & vecPoints, bool bConvex = true)
{
	vecPoints.clear();

	// 处理不同边数的多边形
	if (vecSegPoints.size() == 3)
	{
		vecPoints.push_back(vecSegPoints);
	}
	else if (vecSegPoints.size() >= 4)
	{
		// 五边形及以上：三角剖分
		auto vecTriangles = triangulatePolygon2D(vecSegPoints);
		for (const auto& tri : vecTriangles)
		{
			vecPoints.push_back(tri);
		}
	}
	else
		return false;

	return true;
}

// 判断闭合轮廓中 prev-curr-next 是否在 3D 与 UV 上同时共线（curr 为冗余中间点）
// vEps：3D 叉积长度阈值；uvEps：UV 叉积阈值
static bool IsCollinearRedundantPoint(const SGeometryPoint& prev, const SGeometryPoint& curr, const SGeometryPoint& next,
	float vEps = 1e-5f, double uvEps = 1e-5)
{
	const float abx = curr.vertice.x - prev.vertice.x;
	const float aby = curr.vertice.y - prev.vertice.y;
	const float abz = curr.vertice.z - prev.vertice.z;
	const float acx = next.vertice.x - prev.vertice.x;
	const float acy = next.vertice.y - prev.vertice.y;
	const float acz = next.vertice.z - prev.vertice.z;
	const float crossX = aby * acz - abz * acy;
	const float crossY = abz * acx - abx * acz;
	const float crossZ = abx * acy - aby * acx;
	const float crossLenSq = crossX * crossX + crossY * crossY + crossZ * crossZ;
	if (crossLenSq > vEps * vEps)
		return false;
	const double uvCross = (curr.ptOutView.x - prev.ptOutView.x) * (next.ptOutView.y - prev.ptOutView.y)
		- (curr.ptOutView.y - prev.ptOutView.y) * (next.ptOutView.x - prev.ptOutView.x);
	return std::fabs(uvCross) <= uvEps;
}

// 剔除单个 segment 闭合轮廓上的共线冗余顶点（至少保留 3 点）
// vEps：3D 共线判定叉积长度阈值；uvEps：UV 共线判定叉积阈值
static void RemoveCollinearRedundantPointsInSegment(SSegmentUnitData& segment,
	float vEps = 1e-5f, double uvEps = 1e-5)
{
	std::vector<SGeometryPoint>& pts = segment.vecSegPoints;
	if (pts.size() < 3)
		return;
	bool bRemoved = true;
	while (bRemoved && pts.size() >= 3)
	{
		bRemoved = false;
		const size_t n = pts.size();
		for (size_t i = 0; i < n; ++i)
		{
			const size_t iPrev = (i + n - 1) % n;
			const size_t iNext = (i + 1) % n;
			if (!IsCollinearRedundantPoint(pts[iPrev], pts[i], pts[iNext], vEps, uvEps))
				continue;
			pts.erase(pts.begin() + static_cast<std::ptrdiff_t>(i));
			bRemoved = true;
			break;
		}
	}
}

// 在两点之间插入一点，返回改点的模型数据，采样点插入计算依据为采样位置
static SGeometryPoint InsertPointInSegment(const CPointD& insertPt, const SGeometryPoint& P, const SGeometryPoint& Q)
{
	SGeometryPoint newPt = P;

	// 原两点
	double fTotalLenght = sqrt((P.ptOutView.x - Q.ptOutView.x) * (P.ptOutView.x - Q.ptOutView.x) +
		(P.ptOutView.y - Q.ptOutView.y) * (P.ptOutView.y - Q.ptOutView.y)
	);

	// 除0判断
	if (fTotalLenght == 0)
		return newPt;


	// P离插入点的距离
	double fPLenght = sqrt((P.ptOutView.x - insertPt.x) * (P.ptOutView.x - insertPt.x) +
		(P.ptOutView.y - insertPt.y) * (P.ptOutView.y - insertPt.y)
	);

	double fRatio = fPLenght / fTotalLenght;

	// 三维模型点
	float x3d = (1.0 - fRatio) * P.vertice.x + fRatio * Q.vertice.x;
	float y3d = (1.0 - fRatio) * P.vertice.y + fRatio * Q.vertice.y;
	float z3d = (1.0 - fRatio) * P.vertice.z + fRatio * Q.vertice.z;

	// 输出视图点
	/*float x_ptOut = (1.0 - fRatio) * P.ptOutView.x + fRatio * Q.ptOutView.x;
	float y_ptOut = (1.0 - fRatio) * P.ptOutView.y + fRatio * Q.ptOutView.y;*/

	// 输入视图点
	float x_ptIn = (1.0 - fRatio) * P.ptInView.x + fRatio * Q.ptInView.x;
	float y_ptIn = (1.0 - fRatio) * P.ptInView.y + fRatio * Q.ptInView.y;

	newPt.vertice = DirectX::XMFLOAT3(x3d, y3d, z3d);
	newPt.ptOutView = insertPt;
	newPt.ptInView = CPointD(x_ptIn, y_ptIn);

	return newPt;
}

struct SGeometryRenderData
{
	std::vector<DirectX::XMFLOAT3> vertices;
	std::vector<DirectX::XMFLOAT3> normals;
	std::vector <std::vector<DirectX::XMFLOAT2>>texcoordArrays;
	std::vector<DirectX::XMFLOAT4> tangents;
	std::vector<DirectX::XMFLOAT4> bitangents;
	std::vector<DirectX::XMFLOAT4> colors;

	std::vector<uint32_t> indices32;	//	顶点序列
	std::vector<uint16_t> indices16;

	DirectX::BoundingBox boundingbox;	// 包围盒


	void Reset()
	{
		indices32.clear();
		indices16.clear();

		vertices.clear();
		normals.clear();
		texcoordArrays.clear();
		texcoordArrays.resize(1);
		tangents.clear();
		bitangents.clear();
		colors.clear();
	}

};

struct GeometryData
{
	std::vector<SSegmentUnitData> vecSegNodes;	// 各个切片对应数据

	// 更新模型数据（全部）
	SGeometryRenderData UpdateverticeDatas()
	{
		SGeometryRenderData sRenderData;
		if (vecSegNodes.empty())
			return sRenderData;

		sRenderData.texcoordArrays.resize(1);

		int nModelPolyCount = 0;
		for (const auto& sPreUnit : vecSegNodes)
			nModelPolyCount += sPreUnit.vecSegPoints.size();

		bool bOver16 = nModelPolyCount / 4 * 6 > 65535;

		int nIndexNum = 0;
		int arrIndices[6] = { 0, 1, 2, 2, 3, 0 };		//一个面的索引顺序

		for (auto& sPreUnit : vecSegNodes)
		{
			std::vector<std::vector<SGeometryPoint>> vecPoints;
			bool bRet = TransPoly(sPreUnit.vecSegPoints, vecPoints, sPreUnit.bConvex);
			if (!bRet)
				continue;


			for (const auto &vec : vecPoints)
			{
				for (auto& nodeData : vec)
				{
					sRenderData.vertices.push_back(nodeData.vertice);
					sRenderData.normals.push_back(nodeData.normal);
					sRenderData.texcoordArrays[0].push_back(DirectX::XMFLOAT2(nodeData.ptOutView.x, nodeData.ptOutView.y));
					sRenderData.tangents.push_back(nodeData.tangent);
					//bitangents.push_back(nodeData.bitangent);
					sRenderData.colors.push_back(nodeData.color);
				}

				int nModelPointsSize = vec.size();

				if (nModelPointsSize == 4)
				{
					for (int j = 0; j < 6; j++)
					{
						if (bOver16)
							sRenderData.indices32.push_back(nIndexNum + arrIndices[j]);
						else
							sRenderData.indices16.push_back(nIndexNum + arrIndices[j]);
					}
				}
				else if (nModelPointsSize == 3)
				{
					for (int j = 0; j < 3; j++)
					{
						if (bOver16)
							sRenderData.indices32.push_back(nIndexNum + arrIndices[j]);
						else
							sRenderData.indices16.push_back(nIndexNum + arrIndices[j]);
					}
				}

				nIndexNum += nModelPointsSize;
			}
		}

		//更新包围盒
		if (!sRenderData.vertices.empty())
			DirectX::BoundingBox::CreateFromPoints(sRenderData.boundingbox, sRenderData.vertices.size(), (const DirectX::XMFLOAT3*)sRenderData.vertices.data(), sizeof(DirectX::XMFLOAT3));

		return sRenderData;
	}

	// ---- Rapidjson 保存/加载（用于替换 ModelModel 内的 QJson 中转）----
	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();

		// 保持与旧 QJson 结构一致：
		// { "GeometryData": [ { "SlicePoints": [ ... ] }, ... ] }
		QByteArray geomKeyUtf8 = Json_GeomData.toUtf8();

		rapidjson::Value preSegNodesArr(rapidjson::kArrayType);
		for (const auto& preSegNode : vecSegNodes)
		{
			rapidjson::Value nodeValue(rapidjson::kObjectType);
			preSegNode.SaveDataToRapidJson(nodeValue, allocator);
			preSegNodesArr.PushBack(nodeValue, allocator);
		}

		root.AddMember(
			rapidjson::Value(geomKeyUtf8.constData(), allocator).Move(),
			preSegNodesArr,
			allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& object)
	{
		if (!object.IsObject())
		{
			return false;
		}

		QByteArray geomKeyUtf8 = Json_GeomData.toUtf8();
		if (!object.HasMember(geomKeyUtf8.constData()) || !object[geomKeyUtf8.constData()].IsArray())
		{
			// 兼容旧/异常数据：缺失或类型不匹配时按空几何体处理
			return true;
		}

		const rapidjson::Value& preSegNodesArr = object[geomKeyUtf8.constData()];
		for (rapidjson::SizeType i = 0; i < preSegNodesArr.Size(); ++i)
		{
			if (!preSegNodesArr[i].IsObject())
			{
				continue;
			}

			SSegmentUnitData node;
			if (!node.LoadDataFromRapidJson(preSegNodesArr[i]))
			{
				continue;
			}
			vecSegNodes.push_back(node);
		}

		return true;
	}
};

static DirectX::XMFLOAT3 ComputeAnchorCenter(const SGeometryRenderData& geoRenderData, EPreviewModelAnchorType type)
{
	// 左、右
	float left = geoRenderData.boundingbox.Center.x + geoRenderData.boundingbox.Extents.x;
	float right = geoRenderData.boundingbox.Center.x - geoRenderData.boundingbox.Extents.x;

	// 下、上
	float bottom = geoRenderData.boundingbox.Center.y - geoRenderData.boundingbox.Extents.y;
	float top = geoRenderData.boundingbox.Center.y + geoRenderData.boundingbox.Extents.y;

	// 后、前
	float back = geoRenderData.boundingbox.Center.z + geoRenderData.boundingbox.Extents.z;
	float front = geoRenderData.boundingbox.Center.z - geoRenderData.boundingbox.Extents.z;

	switch (type)
	{
	case EPMAT_GeometryCenter:
		return geoRenderData.boundingbox.Center;
	case EPMAT_TopCenter:
		return DirectX::XMFLOAT3(geoRenderData.boundingbox.Center.x, top, geoRenderData.boundingbox.Center.z);
	case EPMAT_BottomCenter:
		return DirectX::XMFLOAT3(geoRenderData.boundingbox.Center.x, bottom, geoRenderData.boundingbox.Center.z);
	case EPMAT_LeftCenter:
		return DirectX::XMFLOAT3(left, geoRenderData.boundingbox.Center.y, geoRenderData.boundingbox.Center.z);
	case EPMAT_RightCenter:
		return DirectX::XMFLOAT3(right, geoRenderData.boundingbox.Center.y, geoRenderData.boundingbox.Center.z);
	case EPMAT_FrontCenter:
		return DirectX::XMFLOAT3(geoRenderData.boundingbox.Center.x, geoRenderData.boundingbox.Center.y, front);
	case EPMAT_BackCenter:
		return DirectX::XMFLOAT3(geoRenderData.boundingbox.Center.x, geoRenderData.boundingbox.Center.y, back);
	case EPMAT_OriginalCenter:
	default:
		return DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	}

}

static void ApplyAnchorOffsetToGeometry(GeometryData& geo, EPreviewModelAnchorType anchorType)
{
	SGeometryRenderData geoRenderData = geo.UpdateverticeDatas();

	const DirectX::XMFLOAT3 originalAnchor = ComputeAnchorCenter(
		geoRenderData, EPMAT_OriginalCenter);
	const DirectX::XMFLOAT3 currentAnchor = ComputeAnchorCenter(
		geoRenderData, anchorType);
	const DirectX::XMFLOAT3 offset{
		currentAnchor.x - originalAnchor.x,
		currentAnchor.y - originalAnchor.y,
		currentAnchor.z - originalAnchor.z
	};
	if (offset.x == 0.f && offset.y == 0.f && offset.z == 0.f)
		return;

	// 与预览 T(-当前轴心)*T(当前-原始) 一致：顶点平移 +(当前-原始)
	for (SSegmentUnitData& segment : geo.vecSegNodes)
	{
		for (SGeometryPoint& pt : segment.vecSegPoints)
		{
			pt.vertice.x -= offset.x;
			pt.vertice.y -= offset.y;
			pt.vertice.z -= offset.z;
		}
	}
}

