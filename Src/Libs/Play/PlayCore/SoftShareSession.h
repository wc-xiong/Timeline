#pragma once

#include "PlayCoreGlobal.h"
#include "VABaseDecoder.h"

#include <memory>
#include <string>

/*
* 解码会话角色（替代 m_bSharedDecoder + m_bCreatedDecoder 散落 bool）
* - Independent：硬解或门控未命中的独立软解（非「共享失败降级」）
* - SharedCreator：软解复用先到创建者（sticky Seek / 上层音频注册）
* - SharedBind：软解复用后到挂接者（Seek 只清本卡队列）
*
* 不按 Soft/Hard 拆 CVAPlayer 子类：真实复杂度在共享 vs 独立。
* 门控命中后 CreateOrBind 失败视为软件问题：打日志并使 InitPlayer 失败，不做独立软解降级。
*/

enum class EDecoderSession
{
	Independent = 0,
	SharedCreator,
	SharedBind
};

/*
* 软解复用尝试结果（门控 + CreateOrBind）
* - Skipped：门控未命中 → 调用方走 Independent
* - Attached：共享成功
* - Failed：门控命中但失败 → 调用方不得降级，InitPlayer 失败
*/
enum class ESoftShareAttempt
{
	Skipped = 0,
	Attached,
	Failed
};

/*
* 软解复用会话（组合进 CVAPlayer）
* 收拢门控、CreateOrBind、Finalize。
*/

class SoftShareSession
{
public:
	// 多卡 && 软解 && 视频 && shareKey 非空
	static bool IsEligible(const SPlayerResource& pr, EDecoderType decType, bool bNeedVideo);

	/*
	* @brief 门控命中则 CreateOrBind；未命中返回 Skipped
	* @note 失败返回 Failed 并打 no-fallback 日志；调用方不得再走独立软解降级
	*/
	ESoftShareAttempt TryAttachIfEligible(
		SPlayerResource pr,
		std::string& strFilePath,
		EDecoderType nDecodeType,
		bool bNeedVideo,
		bool bLocate,
		std::chrono::milliseconds position,
		std::shared_ptr<CVABaseDecoder>& outDecoder,
		UINT& outDecoderKey);

	// 幂等：摘本卡 PR；计数归零时 UnInit。清空本会话状态并将 decoder 置空。
	void Release(std::shared_ptr<CVABaseDecoder>& decoder, UINT decoderKey);

	bool IsActive() const { return m_session != EDecoderSession::Independent; }
	bool IsBindOnly() const { return m_session == EDecoderSession::SharedBind; }
	bool IsCreator() const { return m_session == EDecoderSession::SharedCreator; }
	EDecoderSession Session() const { return m_session; }

	void Reset();

private:
	EDecoderSession m_session = EDecoderSession::Independent;
	std::string m_shareKey;
};
