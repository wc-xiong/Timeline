#pragma once
#include "CommonGlobal.h"

#include "Common/Common/Maths/Point.h"
#include "Common/Common/Maths/Matrix3x2.h"

#include <array>
#include <vector>
#include <chrono>

enum EGridType
{
	EGT_Horizon = 1,		//横线
	EGT_Vertical,			//竖线
	EGT_RightOblique,		//右斜线
	EGT_LeftOblique,		//左斜线
	EGT_Count,				//类型总数
};

enum ENodeMove
{
	ENM_Forword,
	ENM_Backword,
	ENM_Top,
	ENM_Bottom,
};

struct SGridLinePty
{
	bool bShow = false;
	int nInterval = 64;
	double fLineWidth = 1.0f;
};

struct SScreenGrid
{
public:
	SScreenGrid()
	{
		m_ptGridOffset = CPointD();
		m_bRoll = true;
		m_nTimeInterval = std::chrono::milliseconds(200);
	}

	SScreenGrid(const SScreenGrid& other)
	{
		m_arrGridPty = other.m_arrGridPty;
		m_ptGridOffset = other.m_ptGridOffset;
		m_bRoll = other.m_bRoll;
	}

	SScreenGrid& operator=(const SScreenGrid& other)
	{
		if (this == &other)
			return *this;

		m_arrGridPty = other.m_arrGridPty;
		m_ptGridOffset = other.m_ptGridOffset;
		m_bRoll = other.m_bRoll;
		m_nTimeInterval = other.m_nTimeInterval;
		return *this;
	}

	void SetGridType(int nIndex, bool bType)
	{
		if (nIndex < 0 || nIndex > EGT_Count)
			return;
		m_arrGridPty[nIndex].bShow = bType;
	}

	void SetGridInterval(int nIndex, int nInterval)
	{
		if (nIndex < 0 || nIndex > EGT_Count)
			return;
		if (nInterval < 1)
			return;
		m_arrGridPty[nIndex].nInterval = nInterval;
	}

	void SetGridLineWidth(int nIndex, double fLineWidth)
	{
		if (nIndex < 0 || nIndex > EGT_Count)
			return;
		if (fLineWidth < 1)
			return;
		m_arrGridPty[nIndex].fLineWidth = fLineWidth;
	}

	void SetGridOffset(CPointD ptOffset)
	{
		if (abs(ptOffset.x) > m_arrGridPty[EGT_Vertical].nInterval)
			ptOffset.x = 0;
		if (abs(ptOffset.y) > m_arrGridPty[EGT_Horizon].nInterval)
			ptOffset.y = 0;
		m_ptGridOffset = ptOffset;
	}

	void SetIsGridRoll(bool bStart)
	{
		m_bRoll = bStart;
	}

	void SetRollInterval(std::chrono::milliseconds nInterval)
	{
		m_nTimeInterval = nInterval;
	}

	std::array<SGridLinePty, EGT_Count> GetGridPty()
	{
		return m_arrGridPty;
	}

	CPointD GetGridOffset() const
	{
		return m_ptGridOffset;
	}

	bool GetIsGridRoll() const
	{
		return m_bRoll;
	}

	std::chrono::milliseconds GetRollInterval() const
	{
		return m_nTimeInterval;
	}

private:
	bool							m_bRoll;					// 是否启动网格滚动
	CPointD							m_ptGridOffset;				// 网格绘制偏移
	std::chrono::milliseconds		m_nTimeInterval;			// 时间间隔
	std::array<SGridLinePty, EGT_Count>	m_arrGridPty;				// 1--横线，2--竖线，3--右斜线,4--左斜线
};


enum EGridShowState
{
	EGSS_None = 0,	// 当前未显示网格
	EGSS_Input,	// 输入视图网格
	EGSS_Output,	// 输出视图网格
};


struct SGridStateData
{
	int nGridShowType = EGSS_None;

	SScreenGrid inputGridPty;
	SScreenGrid outputGridPty;

};

class IManipulableObject
{
public:
	virtual ~IManipulableObject() = default;

	virtual double			GetRotateAngle(bool bInput = true) { return 0.0f; }								//旋转角度
	virtual CPointD			GetRotateCenter(bool bInput = true) { return CPointD(); }						//旋转中心
	virtual CPointD			GetRotateControl(bool bInput = true) { return CPointD(); }						//旋转控制点
	virtual CPointD			GetDeleteControl(bool bInput = true, float fradio = 1.0f) { return CPointD(); }	//删除控制点	
	virtual std::vector<CPointD>	GetSizePoint(bool bInput = true) { return std::vector<CPointD>(); }	//缩放控制点

	virtual int		IsPtInPeak(const CPointD& pt, const double &ThresholdValue, bool bInput = true) { return -1; }		//点是否和顶点重合
	virtual int		IsPtInEdge(const CPointD& pt, const double &ThresholdValue, bool bInput = true) { return -1; }		//点是否在边上
	virtual bool	IsPtInSide(const CPointD& pt, bool bInput = true) { return false; }									//点是否在对象内部
};