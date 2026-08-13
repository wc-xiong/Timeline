
#include "DecoderHelper.h"
#include "TimeLinePlayer.h"
#include "Play/Player/VideoPlayer.h"
#include "Play/Player/PicPlayer.h"
#include "Play/Player/StreamPlayer.h"
#include "Play/Player/TextPlayer.h"
#include "Play/Player/ColorTextPlayer.h"
#include "Play/Player/TestChartPlayer.h"
#include "Play/Player/SubtitlePlayer.h"
#include "Play/Player/GifPlayer.h"
#include "Play/StreamMedia/StreamMediaInterface.h"

#include "Common/Common/CommonDefine.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/IPlayerInterface.h"

#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"

#include <functional>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace std::chrono;
using namespace std::placeholders;

std::shared_ptr<CTimeLinePlayer> CDecoderHelper::CreateTimeLineDecoder(
	const SCellDecodeState& cellDecodeState,
	bool bPartialDecode,
	bool bVirtualScreenDecode)
{
	std::shared_ptr<CTimeLinePlayer> pObject = nullptr;

	auto tlId = cellDecodeState.idCell;
	pObject = std::make_shared<CTimeLinePlayer>(tlId, bPartialDecode, bVirtualScreenDecode, true);

	return pObject;
}

std::shared_ptr<IBasePlayer> CDecoderHelper::CreatePlayer(
	const QUuid& tlId,
	const QUuid& layerId,
	const QUuid& mediaId,
	CTimeLinePlayer* pOwner)
{
	if (!pOwner)
		return nullptr;

	auto pMedia = g_pMediaModel->GetMediaNode(mediaId).lock();
	if (!pMedia)
		return nullptr;

	auto pResource = g_pResourceModel->GetResourceNode(pMedia->GetResourceId());
	if (!pResource)
		return nullptr;

	const auto resType = pResource->GetType();
	BasePlayerContext ctx;
	ctx.tlId				= tlId;
	ctx.layerId				= layerId;
	ctx.mediaId				= mediaId;
	ctx.resId				= pMedia->GetResourceId();
	ctx.gpuIndex			= pOwner->GetGpuIndex();
	// 独立路径音频亲和预填；软解复用注册以 Prepare 后 IsSupportAudio 为准
	ctx.bShouldDecodeAudio	= pOwner->ShouldDecodeAudioOnThisGpu(mediaId);
	ctx.bMultiGpusMode		= pOwner->IsMultiGpusMode();
	if (auto pBaseProp = pMedia->GetBaseProperty())
		ctx.decodingMode = pBaseProp->GetDecodingMode();

	std::shared_ptr<IBasePlayer> pPlayer = nullptr;
	switch (resType)
	{
	case ERT_Invalid:
		assert(0);
		break;
	case ERT_Video:
	{
		auto pVideo = std::dynamic_pointer_cast<IVideoResource>(pResource);
		if (pVideo == nullptr)
			return nullptr;

		// 时间线播放时长优先取 CTimeLineObject::m_duration，用于与截取长度比较决定内部循环
		auto mediaPlayTime = milliseconds(pResource->GetDuration());
		if (g_pTimeLineModel)
		{
			if (auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock())
				mediaPlayTime = pObj->GetDuration();
		}
		pPlayer = std::shared_ptr<IBasePlayer>(new CVideoPlayer(pResource,
			ctx, pMedia->GetClipStartTime(),
			(pMedia->GetClipEndTime() - pMedia->GetClipStartTime()),
			mediaPlayTime));

		pPlayer->Prepare();

		// 唯一门槛：软解复用先到创建者 / 独立路径 min-Adapter 等均由 IsSupportAudio 封装
		if (pPlayer->IsSupportAudio())
		{
			SAudioPlayerProp pty;
			pPlayer->GetAudioPty(pty);

			SPlayerId playerId;
			playerId.tlId			= ctx.tlId;
			playerId.layerId		= ctx.layerId;
			playerId.mediaId		= ctx.mediaId;
			playerId.resId			= ctx.resId;
			playerId.bStreamMedia	= ctx.bStreamMedia;

			pOwner->RegisterFileAudioPlayer(pty, playerId);
		}
	}
	break;
	case ERT_Audio:
	{
		auto pAudio = std::dynamic_pointer_cast<IAudioResource>(pResource);
		if (pAudio == nullptr)
			return nullptr;
		auto mediaPlayTime = milliseconds(pResource->GetDuration());
		if (g_pTimeLineModel)
		{
			if (auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock())
				mediaPlayTime = pObj->GetDuration();
		}
		pPlayer = std::shared_ptr<IBasePlayer>(new CVideoPlayer(pResource,
			ctx,
			pMedia->GetClipStartTime(),
			(pMedia->GetClipEndTime() - pMedia->GetClipStartTime()),
			mediaPlayTime));

		pPlayer->Prepare();

		if (pPlayer->IsSupportAudio())
		{
			SAudioPlayerProp pty;
			pPlayer->GetAudioPty(pty);

			SPlayerId playerId;
			playerId.tlId			= ctx.tlId;
			playerId.layerId		= ctx.layerId;
			playerId.mediaId		= ctx.mediaId;
			playerId.resId			= ctx.resId;
			playerId.bStreamMedia	= ctx.bStreamMedia;

			pOwner->RegisterFileAudioPlayer(pty, playerId);
		}
	}
	break;
	case ERT_Picture:
	{
		auto pPic = std::dynamic_pointer_cast<IPictureResource>(pResource);
		if (pPic == nullptr)
			return nullptr;

		if (pResource->GetResourceFormat().toLower() == "gif")
		{
			pPlayer = std::shared_ptr<IBasePlayer>(new CGifPlayer(pResource, ctx, pResource->GetPath()));
		}
		else
		{
			pPlayer = std::shared_ptr<IBasePlayer>(new CPicPlayer(pResource, ctx, pResource->GetPath(), pPic->GetExifOrientation()));
		}
		pPlayer->Prepare();
	}
	break;
	case ERT_TestChart:
	{
		pPlayer = std::shared_ptr<IBasePlayer>(new CTestChartPlayer(pResource, ctx));
		pPlayer->Prepare();
	}
	break;
	case ERT_VideoCapture:
	case ERT_AudioCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
	{
		ctx.bStreamMedia = true;

		pPlayer = std::shared_ptr<IBasePlayer>(new CStreamPlayer(pResource, ctx));
		if (ctx.bShouldDecodeAudio)
		{
			if (auto pNode = std::dynamic_pointer_cast<IStreamMediaPlayer>(pPlayer))
			{
				pNode->InitAudioRenderCallback(
					std::bind(&CTimeLinePlayer::AddResAudioCtrl, pOwner, _1, _2));
			}
		}

		pPlayer->Prepare();
	}
	break;
	case ERT_Subtitle:
	{
		pPlayer = std::shared_ptr<IBasePlayer>(new CSubtitlePlayer(pResource, ctx));
		pPlayer->Prepare();
	}
	break;
	case ERT_Teleprompter:
	break;
	case ERT_Text:
	{
		pPlayer = std::shared_ptr<IBasePlayer>(new CTextPlayer(pResource, ctx, pMedia->GetShareProperty()->GetScrollParam()));
		pPlayer->Prepare();
	}
	break;
	default:
		assert(0);
		break;
	}

	return pPlayer;
}
