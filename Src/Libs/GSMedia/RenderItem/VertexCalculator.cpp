#include "pch.h"
#include "VertexCalculator.h"
#include "Common/Common/TexFrameCommon.h"
#include "Common/Common/MediaCommon.h"
#include <corecrt_math_defines.h>
#include "Common/Common/TransationTypeDefine.h"
#include <algorithm>
#include <random>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

IVertexCalculator* CreateVertexCalculator(int effType)
{
	switch (effType)
	{
	case ETFT_LeftMove:
		return new CVertexCal_LeftMove();
	case ETFT_RightMove:
		return new CVertexCal_RightMove();
	case ETFT_UpMove:
		return new CVertexCal_UpMove();
	case ETFT_DownMove:
		return new CVertexCal_DownMove();
	case ETFT_LeftTopMove:
		return new CVertexCal_LeftTopMove();
	case ETFT_LeftBottomMove:
		return new CVertexCal_LeftBottomMove();
	case ETFT_RightTopMove:
		return new CVertexCal_RightTopMove();
	case ETFT_RightBottomMove:
		return new CVertexCal_RightBottomMove();
	case ETFT_LeftCover:
		return new CVertexCal_LeftCover();
	case ETFT_TopCover:
		return new CVertexCal_TopCover();
	case ETFT_RightCover:
		return new CVertexCal_RightCover();
	case ETFT_BottomCover:
		return new CVertexCal_BottomCover();
	case ETFT_LeftTopCover:
		return new CVertexCal_LeftTopCover();
	case ETFT_RightTopCover:
		return new CVertexCal_RightTopCover();
	case ETFT_LeftBottomCover:
		return new CVertexCal_LeftBottomCover();
	case ETFT_RightBottomCover:
		return new CVertexCal_RightBottomCover();
	case ETFT_LeftTopObliqueCover:
		return new CVertexCal_LeftTopObliqueCover();
	case ETFT_RightTopObliqueCover:
		return new CVertexCal_RightTopObliqueCover();
	case ETFT_LeftBottomObliqueCover:
		return new CVertexCal_LeftBottomObliqueCover();
	case ETFT_RightBottomObliqueCover:
		return new CVertexCal_RightBottomObliqueCover();
	case ETFT_GrillesHorizontal:
		return new CVertexCal_GrillesHorizontal();
	case ETFT_GrillesVertical:
		return new CVertexCal_GrillesVertical();
	case ETFT_LeftRightOpen:
		return new CVertexCal_LeftRightOpen();
	case ETFT_TopBottomOpen:
		return new CVertexCal_TopBottomOpen();
	case ETFT_LeftRightClose:
		return new CVertexCal_LeftRightClose();
	case ETFT_TopBottomClose:
		return new CVertexCal_TopBottomClose();
	case ETFT_MosaicLarge:
	case ETFT_MosaicMiddle:
	case ETFT_MosaicSmall:
		return new CVertexCal_Mosaic();
	case ETFT_RightRotFull:
		return new CVertexCal_RightRotFull();
	case ETFT_RightRotHalf:
		return new CVertexCal_RightRotHalf();
	case ETFT_RightRotQuarter:
		return new CVertexCal_RightRotQuarter();
	case ETFT_LeftRotFull:
		return new CVertexCal_LeftRotFull();
	case ETFT_LeftRotHalf:
		return new CVertexCal_LeftRotHalf();
	case ETFT_LeftRotQuarter:
		return new CVertexCal_LeftRotQuarter();
	case ETFT_SmallToLargeCenter:
		return new CVertexCal_SmallToLargeCenter();
	case ETFT_SmallToLargeLeftTop:
		return new CVertexCal_SmallToLargeLeftTop();
	case ETFT_SmallToLargeLeftBottom:
		return new CVertexCal_SmallToLargeLeftBottom();
	case ETFT_SmallToLargeRightTop:
		return new CVertexCal_SmallToLargeRightTop();
	case ETFT_SmallToLargeRightBottom:
		return new CVertexCal_SmallToLargeRightBottom();
	case ETFT_CenterToBorderRect:
		return new CVertexCal_CenterToBorderRect();
	case ETFT_BorderToCenterRect:
		return new CVertexCal_BorderToCenterRect();
	case ETFT_CenterToBorderLozenge:
		return new CVertexCal_CenterToBorderLozenge();
	case ETFT_BorderToCenterLozenge:
		return new CVertexCal_BorderToCenterLozenge();
	case ETFT_CenterToBorderCross:
		return new CVertexCal_CenterToBorderCross();
	case ETFT_BorderToCenterCross:
		return new CVertexCal_BorderToCenterCross();
	case ETFT_PerspectiveRotationLeft:
		return new CVertexCal_PerspectiveRotationLeft();
	case ETFT_PerspectiveRotationRight:
		return new CVertexCal_PerspectiveRotationRight();
	case ETFT_PerspectiveRotationTop:
		return new CVertexCal_PerspectiveRotationTop();
	case ETFT_PerspectiveRotationBottom:
		return new CVertexCal_PerspectiveRotationBottom();
	default:
		assert(0);
		return nullptr;
	}

	return nullptr;
}

void CVertexCal_LeftMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
}

void CVertexCal_RightMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
	}
}

void CVertexCal_UpMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
	}
}

void CVertexCal_DownMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
}

void CVertexCal_LeftTopMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, fProgress)));
		vecIndexes = { 0,1,2,2,5,0,5,2,3,3,4,5 };
	}
}

void CVertexCal_LeftBottomMove::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecIndexes = { 0,1,2,2,3,4,4,5,2,2,5,0 };
	}
}

void CVertexCal_RightTopMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f - fProgress)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
		vecIndexes = { 0,1,4,4,1,2,2,3,4,4,5,0 };
	}
}

void CVertexCal_RightBottomMove::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecIndexes = { 0,1,3,3,1,2,3,4,5,5,0,3 };
	}
}

void CVertexCal_LeftCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 	}
}

void CVertexCal_RightCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1 - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1 - fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1 - fProgress, 0.0f)));
 	}
}

void CVertexCal_TopCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
 	}
}

void CVertexCal_BottomCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0 - fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0 - fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 	}
}

void CVertexCal_LeftTopCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, fProgress)));
 		vecIndexes = { 0,1,2,2,5,0,5,2,3,3,4,5};
 	}
}

void CVertexCal_RightTopCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
 		vecIndexes = { 0,1,4,4,1,2,2,3,4,4,5,0};
 	}
}

void CVertexCal_LeftBottomCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f - fProgress)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f - fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f , -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f , 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f , 1.0f , RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f , 0.0f)));
 		vecIndexes = { 0,1,2,2,3,4,4,5,2,2,5,0};
 	}
}

void CVertexCal_RightBottomCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f - fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 		vecIndexes = { 0,1,3,3,1,2,3,4,5,5,0,3};
 	}
}

void CVertexCal_LeftTopObliqueCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	vecIndexes.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5)
		{
			float fPosOffset = 2.0f * 2.0f * fProgress;
			float fTexOffset = 2.0f * fProgress;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f + fTexOffset, 0.0f)));
			vecIndexes = { 0,1,2 };
		}
		else
		{
			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5f);
			float fTexOffset = 2.0f * (fProgress - 0.5f);
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f + fTexOffset, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecIndexes = { 0,1,2,2,3,0,3,4,0 };
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress > 0.5)
 		{
 			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5);
 			float fTexOffset = 2.0f * (fProgress - 0.5);
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f + fTexOffset, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f + fTexOffset)));
 			vecIndexes = { 0,1,2 };
 		}
 		else
 		{
 			float fPosOffset = 2.0f * 2.0f * fProgress;
 			float fTexOffset = 2.0f * fProgress;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f + fTexOffset, 0.0f)));
 			vecIndexes = { 0,1,2,2,3,0,3,4,0 };
 		}
 	}
}

void CVertexCal_RightTopObliqueCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	vecIndexes.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5)
		{
			float fPosOffset = 2.0f * 2.0f * fProgress;
			float fTexOffset = 2.0f * fProgress;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f -fTexOffset, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecIndexes = { 0,1,2 };
		}
		else
		{
			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5f);
			float fTexOffset = 2.0f * (fProgress - 0.5f);
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecIndexes = { 0,1,4,4,1,2,2,3,4};
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress > 0.5)
 		{
 			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5);
 			float fTexOffset = 2.0f * (fProgress - 0.5);
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 1.0f)));
 			vecIndexes = { 0,1,2 };
 		}
 		else
 		{
 			float fPosOffset = 2.0f * 2.0f * fProgress;
 			float fTexOffset = 2.0f * fProgress;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fPosOffset, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 0.0f)));
 			vecIndexes = { 0,1,4,4,1,3,3,1,2};
 		}
 	}
}

void CVertexCal_LeftBottomObliqueCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	vecIndexes.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5)
		{
			float fPosOffset = 2.0f * 2.0f * fProgress;
			float fTexOffset = 2.0f * fProgress;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fTexOffset, 1.0f)));
			vecIndexes = { 0,1,2 };
		}
		else
		{
			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5f);
			float fTexOffset = 2.0f * (fProgress - 0.5f);
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f , -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fTexOffset, 0.0f)));
			vecIndexes = { 0,1,4,4,1,3,3,1,2 };
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress > 0.5)
 		{
 			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5);
 			float fTexOffset = 2.0f * (fProgress - 0.5);
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fTexOffset, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecIndexes = { 0,1,2 };
 		}
 		else
 		{
 			float fPosOffset = 2.0f * 2.0f * fProgress;
 			float fTexOffset = 2.0f * fProgress;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fPosOffset - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fTexOffset, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f , 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecIndexes = { 0,1,4,4,1,2,2,3,4 };
 		}
 	}
}

void CVertexCal_RightBottomObliqueCover::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	vecIndexes.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5)
		{
			float fPosOffset = 2.0f * 2.0f * fProgress;
			float fTexOffset = 2.0f * fProgress;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecIndexes = { 0,1,2 };
		}
		else
		{
			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5f);
			float fTexOffset = 2.0f * (fProgress - 0.5f);
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fTexOffset)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecIndexes = { 0,1,3,3,1,2,3,4,0 };
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress > 0.5)
 		{
 			float fPosOffset = 2.0f * 2.0f * (fProgress - 0.5);
 			float fTexOffset = 2.0f * (fProgress - 0.5);
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 0.0f)));
 			vecIndexes = { 0,1,2 };
 		}
 		else
 		{
 			float fPosOffset = 2.0f * 2.0f * fProgress;
 			float fTexOffset = 2.0f * fProgress;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fPosOffset, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fTexOffset, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fPosOffset - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fTexOffset)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecIndexes = { 0,1,2,2,3,0,0,3,4 };
 		}
 	}
}

void CVertexCal_GrillesHorizontal::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	int nRow = 8;

	vecVertex.clear();
	vecIndexes.clear();		//SetSize
	float fProgress = info.fProgress;
	float fPosH = 2.0f / nRow;
	float fTexH = 1.0f / nRow;
	int nIndex = 0;
	if (info.stage == ETFS_In)
	{
		for (int i = 0; i < nRow; i++)
		{
			float fRatio = (i * 1.0f) / nRow;
			float fTop = 1.0f - fRatio * 2;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fTop, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fRatio)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fTop - fPosH * fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fRatio + fTexH * fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fTop - fPosH * fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fRatio + fTexH * fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fTop, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fRatio)));
			vecIndexes.push_back(nIndex);
			vecIndexes.push_back(nIndex + 1);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 3);
			vecIndexes.push_back(nIndex);
			nIndex += 4;
		}
	}
	else if (info.stage == ETFS_Out)
	{
		for (int i = 0; i < 8; i++)
		{
			float fRatio = (i + 1) / 8.0f;
			float fBottom = 1.0f - fRatio * 2;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fBottom + fPosH * (1.0f - fProgress), RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fRatio - fTexH * (1.0f - fProgress))));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fBottom, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fRatio)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fBottom, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fRatio)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fBottom + fPosH * (1.0f - fProgress), RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fRatio - fTexH * (1.0f - fProgress))));
			vecIndexes.push_back(nIndex);
			vecIndexes.push_back(nIndex + 1);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 3);
			vecIndexes.push_back(nIndex);
			nIndex += 4;
		}
	}
}

void CVertexCal_GrillesVertical::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	int nCol = 8;

	vecVertex.clear();
	vecIndexes.clear();
	float fProgress = info.fProgress;
	float fPosH = 2.0f / nCol;
	float fTexH = 1.0f / nCol;
	int nIndex = 0;
	if (info.stage == ETFS_In)
	{
		for (int i = 0; i < nCol; i++)
		{
			float fRatio = (i * 1.0f) / nCol;
			float fLeft = fRatio * 2 - 1.0f;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fLeft, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fLeft, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fLeft + fPosH * fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio + fTexH * fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fLeft + fPosH * fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio + fTexH * fProgress, 0.0f)));
			vecIndexes.push_back(nIndex);
			vecIndexes.push_back(nIndex + 1);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 3);
			vecIndexes.push_back(nIndex);
			nIndex += 4;
		}
	}
	else if (info.stage == ETFS_Out)
	{
		for (int i = 0; i < 8; i++)
		{
			float fRatio = (i + 1) / 8.0f;
			float fRight = fRatio * 2 - 1.0f;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fRight - fPosH * (1.0f - fProgress), 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio - fTexH * (1.0f - fProgress), 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fRight - fPosH * (1.0f - fProgress), -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio - fTexH * (1.0f - fProgress), 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fRight, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fRight, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fRatio, 0.0f)));
			vecIndexes.push_back(nIndex);
			vecIndexes.push_back(nIndex + 1);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 2);
			vecIndexes.push_back(nIndex + 3);
			vecIndexes.push_back(nIndex);
			nIndex += 4;
		}
	}
}

void CVertexCal_LeftRightOpen::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f - fProgress)*0.5f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f - fProgress)*0.5f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f + fProgress)*0.5f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f + fProgress)*0.5f, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f - fProgress)*0.5f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f - fProgress)*0.5f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f + fProgress)*0.5f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f + fProgress)*0.5f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 		vecIndexes = {0,1,2,2,3,0,4,5,6,6,7,4};
 	}
}

void CVertexCal_TopBottomOpen::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, (1.0f - fProgress)*0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, (1.0f + fProgress)*0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, (1.0f + fProgress)*0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, (1.0f - fProgress)*0.5f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, (1.0f - fProgress)*0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, (1.0f - fProgress)*0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f,1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, (1.0f + fProgress)*0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f,-1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, (1.0f + fProgress)*0.5f)));
 		vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4 };
 	}
}

void CVertexCal_LeftRightClose::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f*fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f*fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f*fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f*fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4 };
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f*fProgress, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f*fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f*fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f*fProgress, 0.0f)));
 	}
}

void CVertexCal_TopBottomClose::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f*fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f*fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - 0.5f*fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - 0.5f*fProgress)));
		vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4 };
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f*fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - 0.5f*fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - 0.5f*fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f*fProgress)));
 	}
}

void CVertexCal_Mosaic::PreMosaic(vector<vector<int>>& vecMasaic, int nRow, int nCol, float process)
{
	auto iter = m_mapRandom.find(nRow * nCol);
	if (iter == m_mapRandom.end())
	{
		vector<int> vecRandom;
		vecRandom.resize(nRow * nCol);
		for (int i = 0; i < nRow * nCol; i++)
			vecRandom[i] = i;	//setSize
		std::mt19937 rng(0);  // 这里传入 0 作为种子，和原 srand(0) 效果一致
		std::shuffle(vecRandom.begin(), vecRandom.end(), rng);
		m_mapRandom.insert({ nRow * nCol,vecRandom });
		iter = m_mapRandom.find(nRow * nCol);
	}

	int needCount = int(round(nRow*nCol*process));
	if (needCount <= 0)
		return;

	int hasCount = 0;
	while (needCount > 0)
	{
		int count = iter->second[hasCount++];
		int row = count / nCol;
		int col = count % nCol;
		vecMasaic[col][row] = 1;
		needCount--;
	}
}

void CVertexCal_Mosaic::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	int nRow = 8, nCol = 8;	// Large:8,Middle:12,Samall:16
	if (info.nTransType == ETFT_MosaicMiddle)
		nRow = 12, nCol = 12;
	else if(info.nTransType == ETFT_MosaicSmall)
		nRow = 16, nCol = 16;

	vecVertex.clear();
	vecIndexes.clear();
	float posW = 2.0f / nCol;	//顶点块宽度
	float posH = 2.0f / nRow;	//顶点块高度
	float texW = 1.0f / nCol;	//纹理块宽度
	float texH = 1.0f / nRow;	//纹理块宽度
	int index = 0;

	vector<int> vecTemp;
	vecTemp.resize(nRow, 0);
	vector<vector<int>> vecMosaic;
	vecMosaic.resize(nCol, vecTemp);
	
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		PreMosaic(vecMosaic, nRow, nCol, fProgress);

		for (int i = 0; i < nCol; i++)
		{
			for (int j = 0; j < nRow; j++)
			{
				if (vecMosaic[i][j])
				{
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * i, 1.0f - posH * j, RENDER_ZPLANE), DirectX::XMFLOAT2(i * texW, j * texH)));
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * i, 1.0f - posH * (j + 1), RENDER_ZPLANE), DirectX::XMFLOAT2(i * texW, (j + 1) * texH)));
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * (i + 1), 1.0f - posH * (j + 1), RENDER_ZPLANE), DirectX::XMFLOAT2((i + 1) * texW, (j + 1) * texH)));
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * (i + 1), 1.0f - posH * j, RENDER_ZPLANE), DirectX::XMFLOAT2((i + 1) * texW, j * texH)));
					vecIndexes.push_back(index);
					vecIndexes.push_back(index + 1);
					vecIndexes.push_back(index + 2);
					vecIndexes.push_back(index + 2);
					vecIndexes.push_back(index + 3);
					vecIndexes.push_back(index);
					index += 4;
				}
			}
		}

		if (vecVertex.size() == 0)
		{
			vecVertex = {
	{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
	{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
	{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
	{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };
			vecIndexes = { 0,1,2,2,3,0 };
		}
	}
	else if (info.stage == ETFS_Out)
	{
		PreMosaic(vecMosaic, nRow, nCol, fProgress);

		for (int i = 0; i < nCol; i++)
		{
			for (int j = 0; j < nRow; j++)
			{
				if (!vecMosaic[i][j])
				{
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * i, 1.0f - posH * j, RENDER_ZPLANE), DirectX::XMFLOAT2(i * texW, j * texH)));
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * i, 1.0f - posH * (j + 1), RENDER_ZPLANE), DirectX::XMFLOAT2(i * texW, (j + 1) * texH)));
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * (i + 1), 1.0f - posH * (j + 1), RENDER_ZPLANE), DirectX::XMFLOAT2((i + 1) * texW, (j + 1) * texH)));
					vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f + posW * (i + 1), 1.0f - posH * j, RENDER_ZPLANE), DirectX::XMFLOAT2((i + 1) * texW, j * texH)));
					vecIndexes.push_back(index);
					vecIndexes.push_back(index + 1);
					vecIndexes.push_back(index + 2);
					vecIndexes.push_back(index + 2);
					vecIndexes.push_back(index + 3);
					vecIndexes.push_back(index);
					index += 4;
				}
			}
		}

		if (vecVertex.size() == 0)
		{
			vecVertex = {
	{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
	{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
	{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
	{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };
			vecIndexes = { 0,1,2,2,3,0 };
		}
	}
}

void CVertexCal_RightRotFull::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.125)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 8.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 4.0f, 0.0f)));	//*8.0f表示范围为[0,1],*4.0f表示范围[0,0.5]
			vecIndexes = { 0,1,2};
		}
		else if(fProgress <= 0.375f)
		{
			fProgress -= 0.125;	//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 8.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress * 4.0f)));	//*8.0f表示范围为[0,2],*4.0f表示范围[0,0.5]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

			vecIndexes = { 0,1,2,2,3,0};
		}
		else if (fProgress <= 0.625)
		{
			fProgress -= 0.375;	//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 8.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 4.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

			vecIndexes = { 0,1,4,4,1,2,2,3,4 };
		}
		else if (fProgress <= 0.875)
		{
			fProgress -= 0.625;//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 8.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress * 4.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

			vecIndexes = { 2,3,1,1,3,4,4,5,1,1,5,0 };
		}
		else
		{
			fProgress -= 0.875;//起始校正(0，0.125]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 8.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 4.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

			vecIndexes = { 0,1,6,6,1,5,5,1,4,4,1,3,3,1,2};
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress <= 0.125)
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 8.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 4.0f, 0.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));	
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = { 0,1,5,5,1,2,2,3,5,5,3,4,5,6,0 };
 		}
 		else if (fProgress <= 0.375f)
 		{
 			fProgress -= 0.125;	//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 8.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress * 4.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = { 0,1,4,4,1,2,2,3,4,4,5,0 };
 		}
 		else if (fProgress <= 0.625)
 		{
 			fProgress -= 0.375;	//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 8.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 4.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = { 0,1,2,2,3,0,0,3,4 };
 		}
 		else if (fProgress <= 0.875)
 		{
 			fProgress -= 0.625;//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 8.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress * 4.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = { 0,1,2,2,3,0 };
 		}
 		else
 		{
 			fProgress -= 0.875;//起始校正(0，0.125]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 8.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 4.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));		//*2.0表示fProgress范围为(0,0.5]
 
 			vecIndexes = { 0,1,2 };
 		}
 	}
}

void CVertexCal_RightRotHalf::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.25)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 2.0f, 0.0f)));	//*4.0f表示范围为[0,1],*2.0f表示范围[0,0.5]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 4.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 2.0f, 1.0f)));	
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));

			vecIndexes = { 0,1,2,1,3,4,};
		}
		else if (fProgress <= 0.75f)
		{
			fProgress -= 0.25;	//起始校正(0，0.5]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 4 - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress * 2.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 4, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress * 2.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

			vecIndexes = { 0,1,5,5,6,0,1,2,4,4,2,3};
		}
		else
		{
			fProgress -= 0.75;	//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 2.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 4.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 2.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));


			vecIndexes = { 0,1,6,6,7,0,0,7,8,5,1,2,2,3,5,5,3,4};
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress <= 0.25)
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 4.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 2.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 2.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = { 3,8,0,0,1,3,3,1,2,7,3,6,6,3,5,5,3,4 };
 		}
 		else if (fProgress <= 0.75f)
 		{
 			fProgress -= 0.25;	//起始校正(0，0.5]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 4.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress * 2.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 4.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress * 2.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = { 0,1,2,2,6,0,2,3,4,4,5,2};
 		}
 		else
 		{
 			fProgress -= 0.75;	//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 2.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 4.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 2.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = {0,1,4,1,2,3};
 		}
 	}
}

void CVertexCal_RightRotQuarter::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	float pos = 2.0f / 2;	//顶点块宽度
	float tex = 1.0f / 2;	//纹理块宽度
	int index = 0;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f + fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f , 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress* 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));

			vecIndexes = {0,1,2,3,4,5,6,7,8,9,10,11};
		}
		else
		{
			fProgress -= 0.5f;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f,0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f,1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress* 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress* 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
			vecIndexes = {0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8,12,13,14,14,15,12};
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress > 0.5)
 		{
 			fProgress -= 0.5f;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress* 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress* 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
 			vecIndexes = { 0,1,2,3,4,5,6,7,8,9,10,11 };
 		}
 		else
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress* 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f + fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress* 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
 			vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8,12,13,14,14,15,12 };
 		}
 	}
}

void CVertexCal_LeftRotFull::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.125)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 8.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 4.0f, 0.0f)));	//*8.0f表示范围为[0,1],*4.0f表示范围[0,0.5]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecIndexes = { 0,1,2 };
		}
		else if (fProgress <= 0.375f)
		{
			fProgress -= 0.125;	//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 8.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress * 4.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecIndexes = { 0,1,2,2,3,0 };
		}
		else if (fProgress <= 0.625)
		{
			fProgress -= 0.375;	//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 8.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 4.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecIndexes = { 0,1,2,2,3,0,0,3,4 };
		}
		else if (fProgress <= 0.875)
		{
			fProgress -= 0.625;//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 8.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress * 4.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecIndexes = { 0,1,4,4,1,2,2,3,4,4,5,0};
		}
		else
		{
			fProgress -= 0.875;//起始校正(0，0.125]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 8.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 4.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecIndexes = { 0,1,5,5,1,2,2,3,5,5,3,4,5,6,0};
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress <= 0.125)
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 8.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 4.0f, 0.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 
 			vecIndexes = { 6,0,5,5,0,1,1,2,5,5,2,3,3,4,5};
 		}
 		else if (fProgress <= 0.375f)
 		{
 			fProgress -= 0.125;	//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f,1.0f - fProgress * 8.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress * 4.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 
 			vecIndexes = { 0,1,2,2,5,0,5,2,3,3,4,5};
 		}
 		else if (fProgress <= 0.625)
 		{
 			fProgress -= 0.375;	//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 8.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 4.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 
 			vecIndexes = { 0,1,3,3,1,2,3,4,0};
 		}
 		else if (fProgress <= 0.875)
 		{
 			fProgress -= 0.625;//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 8.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress * 4.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 
 			vecIndexes = { 0,1,2,2,3,0 };
 		}
 		else
 		{
 			fProgress -= 0.875;//起始校正(0，0.125]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 8.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 4.0f, 0.0f)));		//*2.0表示fProgress范围为(0,0.5]
 
 			vecIndexes = { 0,1,2 };
 		}
 	}
}

void CVertexCal_LeftRotHalf::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.25)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 4.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 2.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 2.0f, 1.0f)));	//*4.0f表示范围为[0,1],*2.0f表示范围[0,0.5]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecIndexes = { 0,1,4,1,2,3 };
		}
		else if (fProgress <= 0.75f)
		{
			fProgress -= 0.25;	//起始校正(0，0.5]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 4.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress * 2.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 4 - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress * 2.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecIndexes = { 0,1,2,2,6,0,2,3,4,4,5,2, };
		}
		else
		{
			fProgress -= 0.75;	//起始校正(0，0.25]
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 2.0f, 1.0F)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));	
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 4.0F, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 2.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));


			vecIndexes = { 3,8,0,0,1,3,3,1,2,3,4,5,5,6,3,3,6,7};
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress <= 0.25)
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 2.0f, 1.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 4.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 2.0f, 0.0f)));
 
 			vecIndexes = { 3,8,0,0,1,3,3,1,2,3,4,5,5,6,3,3,6,7};
 		}
 		else if (fProgress <= 0.75f)
 		{
 			fProgress -= 0.25;	//起始校正(0，0.5]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 4.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress * 2.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));//*8.0f表示fProgress范围为[0,1],*4.0f同理
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f,0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 4.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress * 2.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 
 			vecIndexes = {0,1,3,3,1,2,3,4,5,5,6,3};
 		}
 		else
 		{
 			fProgress -= 0.75;	//起始校正(0，0.25]
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 4.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 2.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 4.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 2.0f, 0.0f)));
 
 			vecIndexes = { 0,1,4,1,2,3 };
 		}
 	}
}

void CVertexCal_LeftRotQuarter::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	float pos = 2.0f / 2;	//顶点块宽度
	float tex = 1.0f / 2;	//纹理块宽度
	int index = 0;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0,-fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f + fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));

			vecIndexes = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		}
		else
		{
			fProgress -= 0.5f;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress* 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
			vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8,12,13,14,14,15,12 };
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress > 0.5)
 		{
 			fProgress -= 0.5f;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f,1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecIndexes = { 0,1,2,3,4,5,6,7,8,9,10,11 };
 		}
 		else
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f,0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 0.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f + fProgress)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 1.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f - fProgress)));
 			vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8,12,13,14,14,15,12 };
 		}
 	}
}

void CVertexCal_SmallToLargeCenter::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f - fProgress) * 0.5f, (1.0f - fProgress) * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f - fProgress) * 0.5f, (1.0f + fProgress) * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f + fProgress) * 0.5f, (1.0f + fProgress) * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2((1.0f + fProgress) * 0.5f, (1.0f - fProgress) * 0.5f)));
 		vecIndexes = {0,1,4,4,1,5,1,2,5,5,2,6,2,3,6,6,3,7,3,0,7,7,0,4};
 	}
}

void CVertexCal_SmallToLargeLeftTop::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, fProgress)));

		vecIndexes = { 2,3,4,4,5,2,2,5,0,0,1,2 };
	}
}

void CVertexCal_SmallToLargeLeftBottom::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

		vecIndexes = { 0,1,5,5,1,2,2,3,5,5,3,4 };
	}
}

void CVertexCal_SmallToLargeRightTop::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));

		vecIndexes = { 0,1,5,5,1,4,4,1,3,3,1,2 };
	}
}

void CVertexCal_SmallToLargeRightBottom::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
	else if (info.stage == ETFS_Out)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));

		vecIndexes = { 0,1,2,2,3,0,0,3,4,4,5,0 };
	}
}

void CVertexCal_CenterToBorderRect::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 0.5f, 0.5f - fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 0.5f, 0.5f + fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 0.5f, 0.5f + fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 0.5f, 0.5f - fProgress * 0.5f)));
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 0.5f, 0.5f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress * 0.5f, 0.5f + fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 0.5f, 0.5f + fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress * 0.5f, 0.5f - fProgress * 0.5f)));
 		vecIndexes = { 0,1,4,4,1,5,1,2,5,5,2,6,2,3,6,6,3,7,3,0,7,7,0,4 };
 	}
}

void CVertexCal_BorderToCenterRect::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 0.5f, fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 0.5f, 1.0f - fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, 1.0f - fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, fProgress * 0.5f)));
		vecIndexes = { 0,1,4,4,1,5,1,2,5,5,2,6,2,3,6,6,3,7,3,0,7,7,0,4 };
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 0.5f, fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 0.5f, 1.0f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, 1.0f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, fProgress * 0.5f)));
 	}
}

void CVertexCal_CenterToBorderLozenge::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5f)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f,0.5f + fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 0.5f)));
		}
		else
		{
			fProgress -= 0.5f;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f + fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f + fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 0.0f)));

			vecIndexes = {0,1,2,2,3,0,0,3,4,4,7,0,4,5,6,6,7,4 };
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress <= 0.5f)
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 0.5f + fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 0.5f)));
 
 			vecIndexes = { 0,1,5,5,1,6,6,1,2,2,7,6,2,3,7,7,3,4,4,3,0,0,5,4 };
 		}
 		else
 		{
 			fProgress -= 0.5f;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 0.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f + fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f,-1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f - fProgress, 1.0f)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f + fProgress)));
 
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f + fProgress, 0.0f)));
 
 			vecIndexes = { 0,1,2,3,4,5,6,7,8,9,10,11 };
 		}
 	}
}

void CVertexCal_BorderToCenterLozenge::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		if (fProgress <= 0.5f)
		{
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));

			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));

			vecIndexes = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		}
		else
		{
			fProgress -= 0.5f;
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.5f)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f - fProgress)));
			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.5f)));

			vecIndexes = { 0,1,5,5,1,6,6,1,2,2,7,6,2,3,7,7,3,4,4,3,0,0,5,4 };
		}
	}
 	else if (info.stage == ETFS_Out)
 	{
 		if (fProgress <= 0.5f)
 		{
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f,1.0f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f -  fProgress, 1.0f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.0f)));
 
 			vecIndexes = { 0,1,2,2,3,0,0,3,4,4,7,0,4,5,6,6,7,4 };
 		}
 		else
 		{
 			fProgress -= 0.5f;
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, 1.0f - fProgress * 2.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress * 2.0f - 1.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress, 0.5f)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(0.0f, fProgress * 2.0f - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f, 1.0f - fProgress)));
 			vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress * 2.0f, 0.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress, 0.5f)));
 		}
 	}
}

void CVertexCal_CenterToBorderCross::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 0.5f * (1.0f - fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f * (1.0f - fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f * (1.0f + fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 0.5f * (1.0f + fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 0.5f * (1.0f + fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f * (1.0f +fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f * (1.0f - fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 0.5f * (1.0f - fProgress))));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 0.0f)));
		vecIndexes = {0,1,10,10,11,0,2,3,4,4,1,2,4,5,6,6,7,4,1,4,7,7,10,1,10,7,8,8,9,10};		//六个三角形
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f * (1.0f - fProgress))));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 0.5f * (1.0f - fProgress))));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 0.0f)));
 
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f * (1.0f + fProgress))));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f - fProgress), 0.5f * (1.0f + fProgress))));
 
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 0.5f * (1.0f + fProgress))));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f , 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f * (1.0f + fProgress))));
 
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * (1.0f + fProgress), 0.5f * (1.0f - fProgress))));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f * (1.0f - fProgress))));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f , 0.0f)));
 		vecIndexes = {0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8,12,13,14,14,15,12};
 	}
}

void CVertexCal_BorderToCenterCross::DoCalVertex(STransInfo& info,  vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	vecVertex.clear();
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, fProgress * 0.5f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(fProgress * 0.5f, 0.5f * fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 0.0f)));

		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - 0.5f * fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 1.0f - 0.5f * fProgress)));

		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f * fProgress, 1.0f - 0.5f * fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f * fProgress, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - 0.5f * fProgress)));

		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f * fProgress, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - 0.5f * fProgress, 0.5f * fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f * fProgress)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
		
		vecIndexes = { 0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8,12,13,14,14,15,12 };
	}
 	else if (info.stage == ETFS_Out)
 	{
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 0.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 0.5f * fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.5f * fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 1.0f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(fProgress - 1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.5f * fProgress, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, 1.0f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, 1.0f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, fProgress - 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f - fProgress * 0.5f)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.5f * fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f - fProgress, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, 0.5f * fProgress)));
 		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f - fProgress, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f - fProgress * 0.5f, 0.0f)));
 		
 		vecIndexes = { 0,1,10,10,11,0,2,3,4,4,1,2,4,5,6,6,7,4,1,4,7,7,10,1,10,7,8,8,9,10 };
 	}
}

void CVertexCal_PerspectiveRotationLeft::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.clear();
		double theta = (1 - fProgress) * M_PI_4 * 3;
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(float(cos(theta)*2.0f - 1.0f), -1.0f, float(RENDER_ZPLANE + sin(theta))), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(float(cos(theta)*2.0f - 1.0f), 1.0f, float(RENDER_ZPLANE + sin(theta))), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
}

void CVertexCal_PerspectiveRotationRight::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.clear();
		double theta = fProgress * M_PI_4 * 3 + M_PI_4;
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(float(cos(theta)*2.0f + 1.0f), 1.0f, float(RENDER_ZPLANE + sin(theta))), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(float(cos(theta)*2.0f + 1.0f), -1.0f, float(RENDER_ZPLANE + sin(theta))), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
}

void CVertexCal_PerspectiveRotationTop::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.clear();
		double theta = (1 - fProgress) * M_PI_4 * 3 - M_PI_2;
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, float(sin(theta) * 2.0 + 1.0), float(RENDER_ZPLANE + cos(theta))), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, float(sin(theta) * 2.0 + 1.0), float(RENDER_ZPLANE + cos(theta))), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
}

void CVertexCal_PerspectiveRotationBottom::DoCalVertex(STransInfo& info, vector<VertexPosTex>& vecVertex, vector<DWORD>& vecIndexes)
{
	float fProgress = info.fProgress;
	if (info.stage == ETFS_In)
	{
		vecVertex.clear();
		double theta = fProgress * M_PI_4 * 3 - M_PI_4;
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, float(sin(theta) * 2.0 - 1.0), float(RENDER_ZPLANE + cos(theta))), DirectX::XMFLOAT2(0.0f, 0.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(0.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), DirectX::XMFLOAT2(1.0f, 1.0f)));
		vecVertex.push_back(VertexPosTex(DirectX::XMFLOAT3(1.0f, float(sin(theta) * 2.0 - 1.0), float(RENDER_ZPLANE + cos(theta))), DirectX::XMFLOAT2(1.0f, 0.0f)));
	}
}