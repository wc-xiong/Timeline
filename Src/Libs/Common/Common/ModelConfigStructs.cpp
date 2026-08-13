#include "ModelConfigStructs.h"
#include "CommonFunctionMath.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CContourNode::CContourNode()
{
	m_id = QUuid::createUuid();
}

CContourNode::CContourNode(const CContourNode & other) : m_id(other.m_id)
{
	m_Vertex = other.m_Vertex;
	m_Edge = other.m_Edge;

	m_bCoincide = other.m_bCoincide;
	m_setCrossIds = other.m_setCrossIds;
	m_index = other.m_index;

	m_bGenerateFace = other.m_bGenerateFace;
}

CContourNode::CContourNode(const SBaseVertexProperties& Vetex, const SBaseEdgeProperties& Edge)
	:m_Vertex(Vetex), m_Edge(Edge)
{
	m_id = QUuid::createUuid();
}

bool CContourNode::operator == (const CContourNode& other) const
{
	return m_Vertex == other.m_Vertex && m_Edge == other.m_Edge && m_bGenerateFace == other.m_bGenerateFace;
}

bool CContourNode::IsEnableSmooth()
{
	return m_Vertex.m_bEnableSmooth;
}

void CContourNode::SetEnableSmooth(bool b)
{
	m_Vertex.m_bEnableSmooth = b;
}

bool CContourNode::IsGenerateFace()
{
	return m_bGenerateFace;
}

void CContourNode::SetGenerateFace(bool bGenerate)
{
	m_bGenerateFace = bGenerate;
}

bool CContourNode::IsCoincide()
{
	return m_bCoincide;
}

void CContourNode::SetCoincide(bool b)
{
	m_bCoincide = b;
}

bool CContourNode::IsCross()
{
	return !m_setCrossIds.empty();
}

void CContourNode::AddCross(const QUuid& id)
{
	m_setCrossIds.insert(id);
}

void CContourNode::RemoveCross(const QUuid& id)
{
	m_setCrossIds.erase(id);
}

std::set<QUuid> CContourNode::GetCrossSet()
{
	return m_setCrossIds;
}

void CContourNode::ResetCrossState()
{
	m_setCrossIds.clear();
}

QUuid CContourNode::GetId()
{
	return m_id;
}

void CContourNode::SetIndex(int index)
{
	m_index = index;
}

int CContourNode::GetIndex()
{
	return m_index;
}

void CContourNode::SetEdgeVertexs(const std::vector<CPointD>& vecVertexs)
{
	m_Edge.m_vecDiscretizeVertices = vecVertexs;
}

void CContourNode::GetEdgeVertexs(std::vector<CPointD>& vecVertexs)
{
	vecVertexs = m_Edge.m_vecDiscretizeVertices;
}

void CContourNode::ClearEdgeVertexs()
{
	m_Edge.m_vecDiscretizeVertices.clear();
}

CPointD CContourNode::GetPosVertex()
{
	return m_Vertex.m_posVertex;
}

CPointD CContourNode::GetLeftPosHandle()
{
	return m_Vertex.m_posLeftHandle;
}

CPointD CContourNode::GetRightPosHandle()
{
	return m_Vertex.m_posRightHandle;
}

void CContourNode::SetPosVertex(const CPointD& pos)
{
	m_Vertex.m_posVertex = pos;
	if (!m_Vertex.m_bEnableSmooth)
	{
		m_Vertex.m_posLeftHandle = pos;
		m_Vertex.m_posRightHandle = pos;
	}
}

void CContourNode::SetLeftPosHandle(const CPointD & pos)
{
	m_Vertex.m_posLeftHandle = pos;
}

void CContourNode::SetRightPosHandle(const CPointD & pos)
{
	m_Vertex.m_posRightHandle = pos;
}

int CContourNode::GetEdgeStep()
{
	return m_Edge.m_nSegmentCount;
}

void CContourNode::SetEdgeStep(int n)
{
	m_Edge.m_nSegmentCount = n;
}

double CContourNode::GetLength()
{
	double L = 0.0f;
	for (int i = 0; i < m_Edge.m_vecDiscretizeVertices.size() - 1; i++)
	{
		auto p1 = m_Edge.m_vecDiscretizeVertices[i];
		auto p2 = m_Edge.m_vecDiscretizeVertices[i + 1];

		L += G_Math_CalcLength(p1, p2);
	}

	return L;
}

void DiscretizeBezier(SBaseEdgeProperties& Edge, SBaseVertexProperties& begin, SBaseVertexProperties& end)
{
	Edge.m_vecDiscretizeVertices.clear();

	if (!(begin.m_bEnableSmooth || end.m_bEnableSmooth) || Edge.m_nSegmentCount == 1)
	{
		double dSegmentCount = Edge.m_nSegmentCount;

		Edge.m_vecDiscretizeVertices.push_back(begin.m_posVertex);
		double dIncrementX = (end.m_posVertex.x - begin.m_posVertex.x) / dSegmentCount;
		double dIncrementY = (end.m_posVertex.y - begin.m_posVertex.y) / dSegmentCount;

		CPointD tempPos = begin.m_posVertex;
		for (double i = 1; i < dSegmentCount; i++)
		{
			tempPos.x += dIncrementX;
			tempPos.y += dIncrementY;
			Edge.m_vecDiscretizeVertices.push_back(tempPos);
		}
		Edge.m_vecDiscretizeVertices.push_back(end.m_posVertex);
		return;
	}

	const int segmentCount = Edge.m_nSegmentCount;		// 曲线细分段数	
	const int sampleCount = 1024;						// 曲线弧长采样数，控制精度和表的分辨率

	// Step 1: 使用辛普森积分构建 s(t) 表：弧长与参数 t 的映射
	std::vector<std::pair<double, double>> s_t_table;
	double totalLength = 0.0;
	s_t_table.emplace_back(0.0, 0.0); // 初始点

	for (int i = 1; i <= sampleCount; ++i)
	{
		double a = static_cast<double>(i - 1) / sampleCount;
		double b = static_cast<double>(i) / sampleCount;

		// 辛普森积分参数设置
		int n_simpson = 8; // 每个区间细分次数（偶数）
		double h = (b - a) / n_simpson;
		double integral = 0.0;

		for (int k = 0; k <= n_simpson; ++k)
		{
			double t_k = a + k * h;
			double weight = (k == 0 || k == n_simpson) ? 1 :
				(k % 2 == 1) ? 4 : 2;
			// 计算速度模长 |B'(t)|
			double u = 1.0 - t_k;
			double u2 = u * u;
			double t2 = t_k * t_k;

			CPointD dx(
				begin.m_posVertex.x * (-3 * u2) +
				begin.m_posRightHandle.x * (3 * u2 - 6 * u * t_k) +
				end.m_posLeftHandle.x * (6 * u * t_k - 3 * t2) +
				end.m_posVertex.x * 3 * t2,

				begin.m_posVertex.y * (-3 * u2) +
				begin.m_posRightHandle.y * (3 * u2 - 6 * u * t_k) +
				end.m_posLeftHandle.y * (6 * u * t_k - 3 * t2) +
				end.m_posVertex.y * 3 * t2
			);

			double speed = sqrt(dx.x * dx.x + dx.y * dx.y);
			integral += weight * speed;
		}

		integral *= h / 3.0;
		totalLength += integral;

		s_t_table.emplace_back(totalLength, b);
	}

	// Step 2: 归一化总长度为1，方便后续插值
	for (auto& p : s_t_table)
		p.first /= totalLength;

	// Step 3: 根据等分弧长查找对应的 t 值，并计算坐标
	double desiredSegmentLength = 1.0 / segmentCount;
	Edge.m_vecDiscretizeVertices.push_back(begin.m_posVertex); // 添加起点

	for (int i = 1; i < segmentCount; ++i)
	{
		double targetS = i * desiredSegmentLength;

		// 查找目标位置
		auto it = std::lower_bound(s_t_table.begin(), s_t_table.end(),
			std::make_pair(targetS, 0.0),
			[](const std::pair<double, double>& a,
				const std::pair<double, double>& b)
		{ return a.first < b.first; });

		if (it == s_t_table.begin())
		{
			Edge.m_vecDiscretizeVertices.push_back(begin.m_posVertex);
			continue;
		}
		if (it == s_t_table.end())
		{
			Edge.m_vecDiscretizeVertices.push_back(end.m_posVertex);
			continue;
		}

		--it;
		auto nextIt = it + 1;

		double s0 = it->first;
		double s1 = nextIt->first;
		double t0 = it->second;
		double t1 = nextIt->second;

		double ratio = (targetS - s0) / (s1 - s0);
		double t = t0 + ratio * (t1 - t0);

		// 计算贝塞尔曲线上的点 B(t)
		double u = 1.0 - t;
		double u2 = u * u;
		double u3 = u2 * u;
		double t2 = t * t;
		double t3 = t2 * t;

		CPointD point =
			begin.m_posVertex * u3 +
			begin.m_posRightHandle * 3 * u2 * t +
			end.m_posLeftHandle * 3 * u * t2 +
			end.m_posVertex * t3;

		Edge.m_vecDiscretizeVertices.push_back(point);
	}

	Edge.m_vecDiscretizeVertices.push_back(end.m_posVertex); // 添加终点
}

void DiscretizeBezier(std::shared_ptr<CContourNode> pBeginNode, std::shared_ptr<CContourNode> pEndNode, bool bChangeBeginNode)
{
	if (!pBeginNode || !pEndNode)
	{
		return;
	}

	CPointD beginPosVertex = pBeginNode->GetPosVertex();
	CPointD beginRightHandleVertex = pBeginNode->GetRightPosHandle();

	CPointD endPosVertex = pEndNode->GetPosVertex();
	CPointD endLeftHandleVertex = pEndNode->GetLeftPosHandle();

	if (bChangeBeginNode)
		pBeginNode->ClearEdgeVertexs();
	else
		pEndNode->ClearEdgeVertexs();

	std::vector<CPointD> vecDiscretizeVertices;

	double dSegmentCount = pBeginNode->GetEdgeStep();
	if (!(pBeginNode->IsEnableSmooth() || pEndNode->IsEnableSmooth()) || dSegmentCount == 1)
	{
		vecDiscretizeVertices.push_back(beginPosVertex);
		double dIncrementX = (endPosVertex.x - beginPosVertex.x) / dSegmentCount;
		double dIncrementY = (endPosVertex.y - beginPosVertex.y) / dSegmentCount;

		for (double i = 1; i < dSegmentCount; i++)
		{
			beginPosVertex.x += dIncrementX;
			beginPosVertex.y += dIncrementY;
			vecDiscretizeVertices.push_back(beginPosVertex);
		}
		vecDiscretizeVertices.push_back(endPosVertex);

		if (bChangeBeginNode)
			pBeginNode->SetEdgeVertexs(vecDiscretizeVertices);
		else
			pEndNode->SetEdgeVertexs(vecDiscretizeVertices);

		return;
	}

	const int sampleCount = 1024;						// 曲线弧长采样数，控制精度和表的分辨率

	// Step 1: 使用辛普森积分构建 s(t) 表：弧长与参数 t 的映射
	std::vector<std::pair<double, double>> s_t_table;
	double totalLength = 0.0;
	s_t_table.emplace_back(0.0, 0.0); // 初始点

	for (int i = 1; i <= sampleCount; ++i)
	{
		double a = static_cast<double>(i - 1) / sampleCount;
		double b = static_cast<double>(i) / sampleCount;

		// 辛普森积分参数设置
		int n_simpson = 8; // 每个区间细分次数（偶数）
		double h = (b - a) / n_simpson;
		double integral = 0.0;

		for (int k = 0; k <= n_simpson; ++k)
		{
			double t_k = a + k * h;
			double weight = (k == 0 || k == n_simpson) ? 1 :
				(k % 2 == 1) ? 4 : 2;
			// 计算速度模长 |B'(t)|
			double u = 1.0 - t_k;
			double u2 = u * u;
			double t2 = t_k * t_k;

			CPointD dx(
				beginPosVertex.x * (-3 * u2) +
				beginRightHandleVertex.x * (3 * u2 - 6 * u * t_k) +
				endLeftHandleVertex.x * (6 * u * t_k - 3 * t2) +
				endPosVertex.x * 3 * t2,

				beginPosVertex.y * (-3 * u2) +
				beginRightHandleVertex.y * (3 * u2 - 6 * u * t_k) +
				endLeftHandleVertex.y * (6 * u * t_k - 3 * t2) +
				endPosVertex.y * 3 * t2
			);

			double speed = sqrt(dx.x * dx.x + dx.y * dx.y);
			integral += weight * speed;
		}

		integral *= h / 3.0;
		totalLength += integral;

		s_t_table.emplace_back(totalLength, b);
	}

	// Step 2: 归一化总长度为1，方便后续插值
	for (auto& p : s_t_table)
		p.first /= totalLength;

	// Step 3: 根据等分弧长查找对应的 t 值，并计算坐标
	double desiredSegmentLength = 1.0 / dSegmentCount;
	vecDiscretizeVertices.push_back(beginPosVertex); // 添加起点

	for (int i = 1; i < dSegmentCount; ++i)
	{
		double targetS = i * desiredSegmentLength;

		// 查找目标位置
		auto it = std::lower_bound(s_t_table.begin(), s_t_table.end(),
			std::make_pair(targetS, 0.0),
			[](const std::pair<double, double>& a,
				const std::pair<double, double>& b)
		{ return a.first < b.first; });

		if (it == s_t_table.begin())
		{
			vecDiscretizeVertices.push_back(beginPosVertex);
			continue;
		}
		if (it == s_t_table.end())
		{
			vecDiscretizeVertices.push_back(endPosVertex);
			continue;
		}

		--it;
		auto nextIt = it + 1;

		double s0 = it->first;
		double s1 = nextIt->first;
		double t0 = it->second;
		double t1 = nextIt->second;

		double ratio = (targetS - s0) / (s1 - s0);
		double t = t0 + ratio * (t1 - t0);

		// 计算贝塞尔曲线上的点 B(t)
		double u = 1.0 - t;
		double u2 = u * u;
		double u3 = u2 * u;
		double t2 = t * t;
		double t3 = t2 * t;

		CPointD point =
			beginPosVertex * u3 +
			beginRightHandleVertex * 3 * u2 * t +
			endLeftHandleVertex * 3 * u * t2 +
			endPosVertex * t3;

		vecDiscretizeVertices.push_back(point);
	}

	vecDiscretizeVertices.push_back(endPosVertex); // 添加终点

	if (bChangeBeginNode)
		pBeginNode->SetEdgeVertexs(vecDiscretizeVertices);
	else
		pEndNode->SetEdgeVertexs(vecDiscretizeVertices);
}