#pragma once


#include <chrono>
#include "CommonDefine.h"
#include "ResourceDefine.h"
#include "CommonFunctionMath.h"
#include "VideoDataCommon.h"
#include "QUuidEx.h"

#define RENDER_PRELOAD_TIME			milliseconds(5000)	// 预加载时间ms
#define RENDER_TIME_UINT			1000.0f				// 时间精度 1s = 1000ms
#define RENDER_PPT_PLAY_MAX_TIME	86399000			// PPT播放时长最大值：24H - 86400000ms
#define AlphaLimit					0.02f				// 推子误差精度
#define	CHAR_LENGTH					26
using namespace std::chrono;
/////////////////////////////////////枚举状态/////////////////////////////////////
enum EnFadeState
{
	EFS_Invalid,		// 默认
	EFS_FadeIn,			// 淡入
	EFS_FadeInOK,		// 淡入结束
	EFS_FadeOut,		// 淡出
	EFS_FadeOutOK,		// 淡出结束（意味着释放）
};

#define SWITCH_TIME_DEFAULT 0

// 配合使用在音频播放
struct SPlayerId
{
	// 时间线QUuid
	QUuid tlId;
	// 图层QUuid
	QUuid layerId;
	// 时间线节目单元QUuid
	QUuid mediaId;
	// 素材QUuid, 用于筛选与媒体素材绑定的音频播放器，避免重复创建音频播放器
	QUuid resId;
	// 是否是流媒体素材，用于在DecoderManager获取音频数据时，对流媒体素材进行判断
	bool bStreamMedia;

	SPlayerId()
	{
		bStreamMedia = false;
	}

	SPlayerId(bool _isStreamMedia, const QUuid& _tlId, const QUuid& _layerId, const QUuid& _mediaId, const QUuid& _resId)
		: bStreamMedia(_isStreamMedia), tlId(_tlId), layerId(_layerId), mediaId(_mediaId), resId(_resId)
	{
	}

	QUuid GetFatherId() const
	{
		return tlId;
	}

	// 时间线媒体节目单元
	QUuid GetChildId() const
	{
		return mediaId;
	}

	QUuid GetClipId() const
	{
		return resId;
	}

	bool IsStreamMedia()
	{
		return bStreamMedia;
	}

};

static inline bool operator<(const SPlayerId &l, const SPlayerId &r)
{
	//@see memcmp: https://baike.baidu.com/item/memcmp/5494788?fr=ge_ala

	auto fatherCmp = memcmp(&l.GetFatherId(), &r.GetFatherId(), sizeof(QUuid));
	if (fatherCmp)
		return fatherCmp < 0;

	auto childCmp = memcmp(&l.GetChildId(), &r.GetChildId(), sizeof(QUuid));
	if (childCmp)
		return childCmp < 0;

	return false;
}

enum ERenderDestType
{
	ERDT_General = 0,		// 通用
	ERDT_Text = 1,			// 文本
	ERDT_Audio = 2,			// 音频
	ERDT_Video = 3,			// 视频
	ERDT_UnConnect = 4,		// 未连接
	ERDT_TestChart = 5,		// 测试图
	ERDT_HDRTestChart = 6,	// HDR测试图
	ERDT_Capture = 7,		// 采集卡
	ERDT_Timer = 8,			// 计时器
	ERDT_LuckDraw = 9,		// 抽奖
};

// SWindowLayerInfo 与 SScreenInfo 的公共属性
struct SRectInfo
{
	CPointF	ptCenter;		// 中心位置
	int		width;			// 宽度
	int		height;			// 高度
	double	rotation;		// 旋转角度

	// 获取等价于 RECT_F 结构
	D2D1_RECT_F GetRectF() const
	{
		FLOAT harfW = width / 2.0f;
		FLOAT harfH = height / 2.0f;
		return D2D1::RectF((FLOAT)ptCenter.x - harfW, (FLOAT)ptCenter.y - harfH, (FLOAT)ptCenter.x + harfW, (FLOAT)ptCenter.y + harfH);
	}
	// 获取中心旋转后的矩形路径
	std::vector<CPointF> GetRotationRectF() const
	{
		CPointF ptF[4];
		CPointF ptOut[4];
		float harfW = width / 2.0f;
		float harfH = height / 2.0f;
		ptF[0] = { ptCenter.x - harfW, ptCenter.y - harfH };
		ptF[1] = { ptCenter.x + harfW, ptCenter.y - harfH };
		ptF[2] = { ptCenter.x + harfW, ptCenter.y + harfH };
		ptF[3] = { ptCenter.x - harfW, ptCenter.y + harfH };
		CalcAfterRotatePoint(ptCenter, -1.0f *rotation, ptF, ptOut, 4);
		std::vector<CPointF> vec;
		for (int i = 0; i < 4; ++i)
			vec.push_back(ptOut[i]);
		return vec;
	}

	FLOAT GetLeftF() const { return (FLOAT)ptCenter.x - width / 2.0f; }
	FLOAT GetTopF() const { return (FLOAT)ptCenter.y - height / 2.0f; }
	FLOAT GetRightF() const { return (FLOAT)ptCenter.x + width / 2.0f; }
	FLOAT GetBottomF() const { return (FLOAT)ptCenter.y + height / 2.0f; }

	D2D_POINT_2F GetD2DPtCenter() const { return { ptCenter.x, ptCenter.y }; }
};


struct SDrawObjectInfo
{
	QRectF	rcLayer = { 0, 0, 0, 0 };				// 图层位置
	QSize	szCell = { 0,0 };						// 单元大小
	std::vector<CPointF> vecPts = {};
	int		nFillMode{ EMFT_FullWindow };			// 填充模式
	int		nBlendMode{ 0 };						// 混合模式
	int		nRotateMode = 0;
	int		nFlipMode = 0;
	int		nMode = 0;	//ESectionPlayMode::ECPM_LoopSection
	Video_ClrMetedata	clrMete = {};				// 视频颜色元数据
	//CEffectData effectData = {};

	struct
	{
		CPointF	ptAnchor = CPointF(0, 0);		// 旋转锚点时的锚点
		float x = 0.0f, y = 0.0f, z = 0.0f;
	} rotations;
};

struct SCellSeekInfo
{
	QUuid cellId; // 单元QUuid
	std::chrono::microseconds seekPosition{ 0 }; // 播放进度
};
using VecCellSeekInfo = std::vector<SCellSeekInfo>;

//播放单元记录的可能是时间线本身也有可能记录的是时间线图层媒体单元
//后续会拆分成两个结构体，方便维护
struct SCellTexRenderInfo
{
	QUuid cellID;							// CellID
	QUuid layerID;							// layerID
	QUuid tlID;								// tlID
	float fTextPercent	= 0.0f;									// 淡入淡出百分比
	std::chrono::microseconds startTime{ 0 };					// 播放起始时间点
	std::chrono::microseconds progress{ 0 };					// 播放进度
	EResourceType mClipType		= EResourceType::ERT_Invalid;	// 素材类型
	EnPlayMode nPlayMode		= EnPlayMode::EPM_NORMAL;		// 当前节点模式
	EnPlayState nPlayState		= EnPlayState::EPS_Invalid;		// 当前节点状态
	
};
using VecCellInfo = std::vector<SCellTexRenderInfo>;

//时间线渲染参数
struct STimelineRenderInfo
{
	//时间线进度
	SCellTexRenderInfo tlRenderInfo;

	//各图层中时间线节目单元进度
	std::map<QUuid, VecCellInfo, SUuidCompareFunc> tlLayerClipRenderInfo;
};


struct SCellTexRenderData
{
	SCellTexRenderInfo	texInfo;	// 单元信息
	STextureFrame		videoData;	// 纹理数据帧
};
using VecCellData = std::vector<SCellTexRenderData>;

struct SSoftCtrlState
{
	//ESoftPlayCtrlMode		ctrlMode = ESPCM_Normal;	// 控制模式

	EnPlayMode				ctrlMode = EnPlayMode::EPM_NORMAL;	// 待后续删除暂不使用，仅作为占位符
	EnPlayMode				playMode = EnPlayMode::EPM_NORMAL;	// 播放模式

	BOOL operator==(const SSoftCtrlState& other)
	{
		return (ctrlMode == other.ctrlMode)
			&& (playMode == other.playMode);
	}
};

#if 0
inline SCellPlayState TransformSectionToCellPlayState(EnPlayMode playMode, SSectionSwitchInfo sectionSwitchInfo)
{
	SCellPlayState sc;

	sc.idCell = sectionSwitchInfo.mTlGUID;
	sc.mode = playMode;
	// seek:ms
	sc.seek = std::chrono::duration_cast<std::chrono::milliseconds>(sectionSwitchInfo.mMicroTs).count();

	return sc;
}

inline std::vector<SCellPlayState> TransformSectionIdToPlayState(EnPlayMode playMode, SSectionSwitchInfo sectionSwitchInfo)
{
	std::vector<SCellPlayState> ret;

	SCellPlayState tmp;
	tmp.idCell = sectionSwitchInfo.mTlGUID;
	tmp.mode = playMode;
	// seek:ms
	tmp.seek = std::chrono::duration_cast<std::chrono::milliseconds>(sectionSwitchInfo.mMicroTs).count();
	ret.push_back(std::move(tmp));

	return ret;
}

inline std::vector<SCellPlayState> TransformSectionToPlayState(EnPlayMode playMode, std::shared_ptr<SSectionSwitchInfo> pSectionSwitchInfo)
{
	std::vector<SCellPlayState> ret;

	SCellPlayState tmp;
	tmp.idCell = pSectionSwitchInfo->mTlGUID;
	tmp.mode = playMode;
	// seek:ms
	tmp.seek = std::chrono::duration_cast<std::chrono::milliseconds>(pSectionSwitchInfo->mMicroTs).count();
	ret.push_back(std::move(tmp));

	return ret;
}

inline std::vector<SCellPlayState> TransformIdsToPlayStates(EnPlayMode playMode, const std::vector<QUuid>& ids)
{
	std::vector<SCellPlayState> ret;
	for (auto& id : ids)
	{
		SCellPlayState tmp;
		tmp.idCell = id;
		tmp.mode = playMode;
		ret.push_back(std::move(tmp));
	}
	return ret;
}

inline SCellPlayState TransformCellInfoToPlayState(const SCellTexRenderInfo& cellInfo)
{
	SCellPlayState tmp;
	tmp.idCell = cellInfo.cellID;
	tmp.mode = cellInfo.nPlayMode;
	return tmp;
}

inline std::vector<SCellPlayState> TransformCellInfosToPlayStates(const VecCellInfo& cellInfos)
{
	std::vector<SCellPlayState> ret;

	for (auto& cellInfo : cellInfos)
	{
		ret.push_back(TransformCellInfoToPlayState(cellInfo));
	}
		
	return ret;
}
#endif
