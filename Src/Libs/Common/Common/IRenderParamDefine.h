#pragma once

#include <QUuid>
#include <QRect>
#include <QRgb>
//#include <dcommon.h>
#include <DirectXMath.h>
#include <wrl/client.h>		// 为 Microsoft::WRL::ComPtr
#include <d3d11.h>			// 为 ID3D11Buffer
#include <chrono>
#include <vector>
#include <list>
#include <mutex>
#include "Common/Common/Maths/Point.h"
#include "Common/Common/AudioCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/ResourceDefine.h"
//#include "Common/Common/ISliceDataInterface.h"
#include "GSMedia/RenderCommon/Vertex.h"

struct STextureRenderParam
{
	// 颜色空间数据
	Video_ClrMetedata mColorMeteData;
	// 颜色效果数据
	SColorPty mCellClr;

	// 纹理图层在舞台的位置（未旋转）
	D2D1_RECT_F mLayerPosition{ 0.0f, 0.0f, 1920.0f, 1080.0f };
	// 纹理在舞台的位置（未旋转）
	D2D1_RECT_F mTexturePosition{ 0.0f, 0.0f, 1920.0f, 1080.0f };
	// 旋转矩阵
	DirectX::XMMATRIX mRotation = DirectX::XMMatrixIdentity();

	// 混合模式
	int mBlendMode = 0;
	// 填充模式
	EMediaFillType mFillMode = EMFT_KeepRatio;
	// 纹理大小
	QSize mSizeCell = { 0,0 };

	// 纹理内部旋转角度
	int m_nRotate;

	// 纹理内部翻转
	EMediaClipFlipMode mTextureInternalFlip = ECFM_NoFlip;

	// 是否隐藏
	bool mIsHide = false;

	// 是否solo
	bool mIsSolo = false;
};

struct SClipRenderInfo
{
	//播放素材的类型
	EResourceType mClipType = EResourceType::ERT_Invalid;
	QString mClipName;
};

struct STextureTreeNodeParam
{
	// 播放单元Id
	QUuid mCellId;
	// 图层Id
	QUuid mLayerId;
	// 时间线Id
	QUuid mTlId;
	// 播放模式
	EnPlayMode mPlayMode = EnPlayMode::EPM_INVALID;
	// 播放进度
	std::chrono::microseconds mPlayProgress{ 0 };
	//播放状态
	EnPlayState mPlayState = EnPlayState::EPS_Invalid; // 当前节点状态
	//播放素材的信息
	SClipRenderInfo mClipInfo;
};

struct STextClipRenderParam
{
	//背景是否不透明
	bool mOpaque = false;
	//纹理坐标与滚动区域坐标比例（归一化比例）
	float mRatioX = 0.0f, mRatioY = 0.0f;
	//滚动类型
	EScrollType mScrollType = EScrollType::EST_Stop;
	//滚动速度
	float mScrollSpeed = 0.0f;
	//滚动区域
	QRect mScrollRect;
	//总宽高（纹理总宽高+首尾间距）
	int mTotalWidth = 0, mTotalHeight = 0;
	//首尾相接距离
	int mSpaceX = 0, mSpaceY = 0;
	//背景颜色
	QRgb mColorBG = 0;
};

struct SScreenItemRenderParam
{
	// ScreenItem的颜色效果数据
	SColorPty mScreenClr;
	// ScreenItem在舞台的位置
	D2D1_RECT_F mStagePosition{ 0.0f, 0.0f, 1920.0f, 1080.0f };
};

struct SMonitorRenderParam
{
	bool mIsHDR = false;
	bool mIsMSAA = false;
	bool mIsFlip = true;
	bool mIsLowLatency = true;
	bool mIsDiscard = false;
};

struct SGlobalRenderParam
{
	// 上一次渲染的时间戳
	std::chrono::microseconds mLastPts{ 0 };

	// 当前时间戳
	std::chrono::microseconds mPts{ 0 };
	
	// 背景颜色
	DWORD mBackgroundColor = 0;
	// hdr设置
	int mHdr = 0;

	// 全局颜色效果数据
	SColorPty mGlobalClr;
	// 是否黑屏
	bool mIsBlackWnd = false;
};

struct STimelineClipLayerRenderInfo
{
	//窗口顶点可能是通过矩阵变换过的，因此不一定是矩形
	std::vector<QPointF> mTimelinePoints;
	QPointF mPtAnchor;

	// 边框颜色
	QRgb layoutClr;

	// 时间位置
	std::chrono::milliseconds mTimelineClipCellPos{ 0 };
	// 播放总时长
	std::chrono::milliseconds mTimelineClipLength{ 0 };

	// 指示选中
	bool mIsSelected = false;
	// 指示信息展示
	bool mIsShowInfo = false;
	// 指示是否含有旋转特效
	bool mhaveRotationFx = false;

	//时间线-图层名称
	QString mTimelineAndLayerName;
};

struct SCustomGridParam
{
	QUuid screenID;						//屏幕ID
	QRect inScreen;						//输入屏幕区域
	//SScreenGrid* inGrid;				//输入网格数据(数据副本)
	//SScreenGrid* outGrid;				//输出网格数据(数据副本)
};

struct SCustomSlicesParam
{
	QUuid screenID;													//屏幕ID
	bool origin;													//是否为初始创意显示（只有一个初始切片）
	QRect inScreen;													//输入屏幕区域
	QRect outScreen;												//输出屏幕区域
	//EQuadOptimizationMode mode = EQuadOptimizationMode::NoneOpt;	//优化模式
	int mode = 0;	//优化模式

	std::list<std::vector<CPoint3F>> arrTriangles;					//三角形顶点
	std::list<std::vector<CPoint3F>> arrQuadrangles;				//四变形顶点
};

struct SCustomRenderParam
{
	std::mutex								m_sliceMutex;					// 顶点切片锁
	std::vector<VertexPosTex>				m_vecSliceVertex;				// 切片顶点数据
	std::vector<uint32_t>					m_vecSliceIndex;				// 切片索引数据
	std::vector<CPoint3F>					m_vecSliceTexPos;				// 临时存储切片顶点输入位置
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pSliceVertexBuffer;			// 切片顶点缓冲区
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pSliceIndexBuffer;			// 切片索引缓冲区

	void Clear()
	{
		m_vecSliceVertex.clear();
		m_vecSliceIndex.clear();
		m_vecSliceTexPos.clear();
	}
};