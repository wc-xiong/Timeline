#pragma once
#include "CommonGlobal.h"
#include "Common/Common/Maths/Point.h"

#include <array>
#include <vector>
#include <QString>
#include <QUuid>
#include <set>
#include <deque>
#include <map>
#include <memory>

struct SBaseVertexProperties
{
	CPointD m_posVertex;			//当前顶点本身的坐标,单位：mm
	CPointD	m_posLeftHandle;		//以当前顶点为终点的贝塞尔曲线的第二控制点,单位：mm
	CPointD	m_posRightHandle;		//以当前顶点为起点的贝塞尔曲线的第一控制点,单位：mm
	bool m_bEnableSmooth = false;	//是否是平滑点

	bool operator==(const SBaseVertexProperties& other) const
	{
		return m_posVertex == other.m_posVertex &&
			m_posLeftHandle == other.m_posLeftHandle &&
			m_posRightHandle == other.m_posRightHandle &&
			m_bEnableSmooth == other.m_bEnableSmooth;
	}

};

struct SBaseEdgeProperties
{
	int	m_nSegmentCount = 0;						// 贝塞尔曲线的离散化步数
	std::vector<CPointD> m_vecDiscretizeVertices;	// 离散化后的顶点集合

	// 比较运算符
	bool operator==(const SBaseEdgeProperties& other) const
	{
		return m_nSegmentCount == other.m_nSegmentCount;
	}
};

class COMMON_EXPORT CContourNode
{
public:
	CContourNode();
	CContourNode(const CContourNode& other);
	CContourNode(const SBaseVertexProperties& Vetex, const SBaseEdgeProperties& Edge);

	virtual ~CContourNode() {}

	bool operator == (const CContourNode& other) const;

public:
	QUuid GetId();

	void SetIndex(int index);
	int GetIndex();

	bool IsEnableSmooth();
	void SetEnableSmooth(bool b);

	bool IsGenerateFace();
	void SetGenerateFace(bool bGenerate);

	bool IsCoincide();
	void SetCoincide(bool b);

	bool IsCross();
	void AddCross(const QUuid& id);
	void RemoveCross(const QUuid& id);
	std::set<QUuid> GetCrossSet();
	void ResetCrossState();

	void SetEdgeVertexs(const std::vector<CPointD>& vecVertexs);
	void GetEdgeVertexs(std::vector<CPointD>& vecVertexs);
	void ClearEdgeVertexs();

	CPointD GetPosVertex();
	CPointD GetLeftPosHandle();
	CPointD GetRightPosHandle();
	void SetPosVertex(const CPointD& pos);
	void SetLeftPosHandle(const CPointD& pos);
	void SetRightPosHandle(const CPointD& pos);

	int GetEdgeStep();
	void SetEdgeStep(int n);

	double GetLength();

protected:
	QUuid m_id;
	int m_index = -1;

	SBaseVertexProperties m_Vertex;
	SBaseEdgeProperties m_Edge;

	bool m_bCoincide = false;				//是否和其他顶点重合，重合算为交叉
	std::set<QUuid> m_setCrossIds;		//底面边和其他边交叉

	bool m_bGenerateFace = true;
};

struct SContourPath
{
	std::deque<std::shared_ptr<CContourNode>> m_dequeGraphNode;	//绘制轮廓
	std::map<QUuid, std::shared_ptr<CContourNode>> m_mapGraphNode; //方便存取

	std::atomic_bool m_bContourClose{ false };
	std::atomic_bool m_bContourContinuous{ false };
	std::atomic_bool m_bContourCross{ false };
	std::atomic_bool m_bContourCoincide{ false };
	std::atomic_bool m_bContourSelfInteract{ false };
	std::atomic_bool m_bContourConvex{ false };

	SContourPath() = default;

	SContourPath(const SContourPath& other)
	{
		m_dequeGraphNode = other.m_dequeGraphNode;
		m_mapGraphNode = other.m_mapGraphNode;
		m_bContourClose.store(other.m_bContourClose.load());
		m_bContourCross.store(other.m_bContourCross.load());
		m_bContourCoincide.store(other.m_bContourCoincide.load());
		m_bContourContinuous.store(other.m_bContourContinuous.load());
		m_bContourSelfInteract.store(other.m_bContourSelfInteract.load());
		m_bContourConvex.store(other.m_bContourConvex.load());
	}

	SContourPath& operator =(const SContourPath& other)
	{
		m_dequeGraphNode = other.m_dequeGraphNode;
		m_mapGraphNode = other.m_mapGraphNode;
		m_bContourClose.store(other.m_bContourClose.load());
		m_bContourCross.store(other.m_bContourCross.load());
		m_bContourCoincide.store(other.m_bContourCoincide.load());
		m_bContourContinuous.store(other.m_bContourContinuous.load());
		m_bContourSelfInteract.store(other.m_bContourSelfInteract.load());
		m_bContourConvex.store(other.m_bContourConvex.load());

		return *this;
	}

	bool operator ==(const SContourPath& other)
	{
		if (m_dequeGraphNode.size() != other.m_dequeGraphNode.size())
			return false;

		bool bEqual = std::equal(m_dequeGraphNode.begin(), m_dequeGraphNode.end(), other.m_dequeGraphNode.begin(),
				[](const std::shared_ptr<CContourNode>& p1, const std::shared_ptr<CContourNode>& p2)
		{
			if (!p1 && !p2) return true;
			if (!p1 || !p2) return false;
			return *p1 == *p2;
		});


		return bEqual && m_bContourClose.load() == other.m_bContourClose.load()
			&& m_bContourCross.load() == other.m_bContourCross.load()
			&& m_bContourCoincide.load() == other.m_bContourCoincide.load()
			&& m_bContourContinuous.load() == other.m_bContourContinuous.load()
			&& m_bContourSelfInteract.load() == other.m_bContourSelfInteract.load()
			&& m_bContourConvex.load() == other.m_bContourConvex.load();
	}

	void DeepCopy(const SContourPath& other)
	{
		m_dequeGraphNode.clear();
		m_mapGraphNode.clear();

		for (const auto& pNode : other.m_dequeGraphNode)
		{
			if (!pNode)
			{
				continue;
			}

			auto pCopy = std::make_shared<CContourNode>(*pNode);
			m_dequeGraphNode.push_back(pCopy);
			m_mapGraphNode[pCopy->GetId()] = pCopy;
		}

		m_bContourClose.store(other.m_bContourClose.load());
		m_bContourCross.store(other.m_bContourCross.load());
		m_bContourCoincide.store(other.m_bContourCoincide.load());
		m_bContourContinuous.store(other.m_bContourContinuous.load());
		m_bContourSelfInteract.store(other.m_bContourSelfInteract.load());
		m_bContourConvex.store(other.m_bContourConvex.load());
	}
};

//沉浸式平面的面属性
struct SFaceProperties
{
	bool m_bPrensent = true;		//单面屏是否存在

	int m_nWidthDpi = 500;			//分辨率的宽，物理大小的宽/点间距
	int m_nHeightDpi = 500;			//分辨率的高，物理大小的高/点间距

	//二维坐标系下单面屏矩形形状参数，单位mm，初始值为1000
	int m_nTop = 0;
	int m_nBottom = 1000;
	int m_nLeft = 0;
	int m_nRight = 1000;

	float m_dDotPitch = 2.0;		//点间距

	 // 比较运算符
	bool operator!=(const SFaceProperties& other) const {
		return m_bPrensent != other.m_bPrensent ||
			m_nWidthDpi != other.m_nWidthDpi ||
			m_nHeightDpi != other.m_nHeightDpi ||
			m_nTop != other.m_nTop ||
			m_nBottom != other.m_nBottom ||
			m_nLeft != other.m_nLeft ||
			m_nRight != other.m_nRight ||
			m_dDotPitch != other.m_dDotPitch;
	}

	std::array<int, 4> GetFaceAdjustmentArray()
	{
		return std::array<int, 4>{m_nTop, m_nBottom, m_nLeft, m_nRight};
	}
};

// 柱体贝塞尔曲线离散化后的顶点集合
void COMMON_EXPORT DiscretizeBezier(SBaseEdgeProperties& Edge, SBaseVertexProperties& begin, SBaseVertexProperties& end);
void COMMON_EXPORT DiscretizeBezier(std::shared_ptr<CContourNode> pBeginNode, std::shared_ptr<CContourNode> pEndNode, bool bChangeBeginNode = true);

//模组推断模型的模组信息
struct SModuleNode
{
	unsigned int        prcIndex;       // 发送器序号
	unsigned int        portIndex;      // 网口序号
	unsigned int        rcvIndex;       // 在此网口下接收卡序号

	// 模组信息
	int                 x = 0;              // 位置x
	int                 y = 0;              // 位置y
	int                 width = 0;          // 宽
	int                 height = 0;         // 高

	//抽点表信息
	QString strSheetPath;
	std::vector<std::vector<bool>> DrawPointSheet;

	bool operator==(const SModuleNode& other) const
	{
		// 先比较基础数据类型成员
		if (x != other.x ||
			y != other.y ||
			width != other.width ||
			height != other.height ||
			strSheetPath != other.strSheetPath)
		{
			return false;
		}

		// 比较二维vector容器
		// 先检查外层容器大小是否相等
		if (DrawPointSheet.size() != other.DrawPointSheet.size())
		{
			return false;
		}

		// 逐个比较内层容器
		for (size_t i = 0; i < DrawPointSheet.size(); ++i)
		{
			// 检查内层容器大小是否相等
			if (DrawPointSheet[i].size() != other.DrawPointSheet[i].size())
			{
				return false;
			}

			// 比较内层容器的每个元素
			if (!std::equal(DrawPointSheet[i].begin(), DrawPointSheet[i].end(),
				other.DrawPointSheet[i].begin()))
			{
				return false;
			}
		}

		// 所有成员都相等
		return true;
	}

	// 通常也会同时实现!=运算符
	bool operator!=(const SModuleNode& other) const
	{
		return !(*this == other);
	}
};

/**
	 * @brief 拓扑数据（仅邻接关系）
	 */
struct SyntopyData
{
	// n*4的二维数组，第二维度是四个边相邻的网格mesh的下标,及其边的下标（上-0，右-1，下-2，左-3 ）
	std::vector<std::vector<std::pair<int, int>>> allAdjacentMeshIndices;
	
	// 重载==运算符
	bool operator==(const SyntopyData& other) const
	{
		// 比较两个对象的allAdjacentMeshIndices是否完全相等
		return allAdjacentMeshIndices == other.allAdjacentMeshIndices;
	}

	// 通常建议同时重载!=运算符（可选，但符合惯例）
	bool operator!=(const SyntopyData& other) const
	{
		return !(*this == other);
	}
};