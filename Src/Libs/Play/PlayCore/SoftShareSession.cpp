#include "SoftShareSession.h"
#include "SoftDecoderManager.h"

#include <Windows.h>
#include "Common/Common/CommonNonPlatform.h"

bool SoftShareSession::IsEligible(const SPlayerResource& pr, EDecoderType decType, bool bNeedVideo)
{
	return pr.bMultiGpusMode
		&& decType == EDT_SOFTWARW
		&& bNeedVideo
		&& !pr.shareKey.empty();
}

ESoftShareAttempt SoftShareSession::TryAttachIfEligible(
	SPlayerResource pr,
	std::string& strFilePath,
	EDecoderType nDecodeType,
	bool bNeedVideo,
	bool bLocate,
	std::chrono::milliseconds position,
	std::shared_ptr<CVABaseDecoder>& outDecoder,
	UINT& outDecoderKey)
{
	if (!IsEligible(pr, nDecodeType, bNeedVideo))
		return ESoftShareAttempt::Skipped;

	bool bCreated = false;
	auto pDecoder = PlayCore::CSoftDecoderManager::Instance()->CreateOrBind(
		QString::fromStdString(pr.shareKey), pr, strFilePath, nDecodeType,
		bNeedVideo, bLocate, position, bCreated);

	if (!pDecoder)
	{
		// 门控已命中：失败视为软件问题，禁止降级独立软解（避免掩盖 CreateOrBind/Init 故障）
		LOGA() << "SoftShare: act=NoFallback key= " << pr.shareKey
			<< " gpu= " << pr.gpuIndex
			<< " path= " << strFilePath
			<< " tid= " << GetCurrentThreadId();
		Reset();
		return ESoftShareAttempt::Failed;
	}

	m_shareKey = pr.shareKey;
	m_session = bCreated ? EDecoderSession::SharedCreator : EDecoderSession::SharedBind;
	outDecoder = pDecoder;
	outDecoderKey = pr.gpuIndex;
	return ESoftShareAttempt::Attached;
}

void SoftShareSession::Release(std::shared_ptr<CVABaseDecoder>& decoder, UINT decoderKey)
{
	if (!IsActive())
		return;

	std::string key = std::move(m_shareKey);
	Reset();

	auto held = std::move(decoder);
	decoder.reset();

	if (held)
	{
		PlayCore::CSoftDecoderManager::Instance()->Finalize(
			QString::fromStdString(key), held, decoderKey);
	}
}

void SoftShareSession::Reset()
{
	m_session = EDecoderSession::Independent;
	m_shareKey.clear();
}
