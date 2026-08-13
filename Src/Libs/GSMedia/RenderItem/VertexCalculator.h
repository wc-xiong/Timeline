#pragma once
#include "VertexCalculator.h"
#include "GSMedia/RenderCommon/Vertex.h"

#include <map>
#include <vector>

using std::vector;
struct STransInfo;
class IVertexCalculator
{
public:
	IVertexCalculator(void) {};
	virtual ~IVertexCalculator(void) {};

	virtual void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes) = 0;
};

IVertexCalculator* CreateVertexCalculator(int effType);

class CVertexCal_LeftMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_UpMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_DownMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftTopMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftBottomMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightTopMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightBottomMove : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_TopCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_BottomCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftTopCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightTopCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftBottomCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightBottomCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftTopObliqueCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightTopObliqueCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftBottomObliqueCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightBottomObliqueCover : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_GrillesHorizontal : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_GrillesVertical : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftRightOpen : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_TopBottomOpen : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftRightClose : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_TopBottomClose : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_Mosaic : public IVertexCalculator
{
protected:
	void PreMosaic(vector<vector<int>>& vecMasaic, int nRow, int nCol , float process);

	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
private:
	std::map<int,std::vector<int>> m_mapRandom;	//随机序列	
};

class CVertexCal_RightRotFull : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightRotHalf : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_RightRotQuarter : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftRotFull : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftRotHalf : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_LeftRotQuarter : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_SmallToLargeCenter : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_SmallToLargeLeftTop : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_SmallToLargeLeftBottom : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_SmallToLargeRightTop : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_SmallToLargeRightBottom : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_CenterToBorderRect : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_BorderToCenterRect : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_CenterToBorderLozenge : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_BorderToCenterLozenge : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_CenterToBorderCross : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_BorderToCenterCross : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_PerspectiveRotationLeft : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_PerspectiveRotationRight : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_PerspectiveRotationTop : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};

class CVertexCal_PerspectiveRotationBottom : public IVertexCalculator
{
protected:
	virtual	void DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes);
};