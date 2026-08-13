#pragma once

#include <memory>
#include <QUuid>
#include "Common/Common/IDecodeCtrlDefine.h"
#include "Common/Common/IPlayerInterface.h"

class CTimeLinePlayer;
class IBasePlayer;

class CDecoderHelper
{
public:
	CDecoderHelper() = default;
	~CDecoderHelper() = default;
	CDecoderHelper(const CDecoderHelper&) = delete;
	CDecoderHelper& operator=(const CDecoderHelper&) = delete;

public:

	static std::shared_ptr<CTimeLinePlayer> CreateTimeLineDecoder(
		const SCellDecodeState& cellDecodeState,
		bool bPartialDecode,
		bool bVirtualScreenDecode = true);

	static std::shared_ptr<IBasePlayer> CreatePlayer(
		const QUuid& tlId,
		const QUuid& layerId,
		const QUuid& mediaId,
		CTimeLinePlayer* pOwner);
};
