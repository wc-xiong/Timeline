#pragma once

#include "PlayCoreGlobal.h"
#include "VABaseDecoder.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <QString>

/*
* 软解解码器共享管理器（先来后到）
*
* 设计见 Doc/软解复用设计修改版-先来后到方案.md：
* - 多显卡模式下软解素材跨卡复用：先到者创建真实 CVADecoder 并入表，
*   后到者命中后仅 AddPlayerResource 挂接，无 Owner/Consumer 角色；
* - 音频出声先来后到：管理器为每个入表 PR 分配到达序号，解码器内部
*   按最小到达序号决定出声，Owner 退出自动移交次早到达者（见 CVADecoder）；
* - Seek 写者 = 建播创建者（sticky），与音频归属解耦；Bind 卡仅清本卡队列；
* - 生命周期终点：DisplayerPlayerManager::UnInit 调 Shutdown() 全清。
* - 调用方为 SoftShareSession（组合进 CVAPlayer）；不引入 CVASoftPlayer/CVAHardwarePlayer。
*/

namespace PlayCore
{

class PLAYCORE_EXPORT CSoftDecoderManager
{
public:
	// 懒加载全局单例
	static std::shared_ptr<CSoftDecoderManager> Instance();

	// 进程段结束：ClearAll 后销毁单例。由 DisplayerPlayerManager::UnInit 调用。
	static void Shutdown();

	/*
	* @brief 先来后到获取共享软解码器
	* @param key     共享键 layerId|mediaId|filePath
	* @param pr      本卡资源（回调指针必须已填好）
	* @param filePath 媒体文件路径
	* @param decType  解码类型（共享路径恒为 EDT_SOFTWARW）
	* @param bNeedVideo 是否解码视频
	* @param bLocate/position 截取起点
	* @param bCreated 未命中创建时写 true，命中挂接写 false；失败时不写
	* @return 命中 → 现有实例（已追加本卡 PR）；未命中 → 新建并入表；失败 → nullptr
	*         （调用方不得降级独立软解：InitPlayer 应失败；见 SoftShareSession::TryAttachIfEligible）
	* @note 音频流恒开启；为本卡 PR 分配到达序号，出声归属按先来后到（最小到达序号）
	*/
	std::shared_ptr<CVABaseDecoder> CreateOrBind(
		const QString& key,
		SPlayerResource pr,
		const std::string& filePath,
		EDecoderType decType,
		bool bNeedVideo,
		bool bLocate,
		std::chrono::milliseconds position,
		bool& bCreated);

	/*
	* @brief 释放参与者：摘本卡 PR；PR 计数归零时摘表并 UnInit（锁外执行）
	* @param key      共享键
	* @param decoder  参与者持有的解码器（校验一致性，防误摘新条目）
	* @param gpuIndex 本卡物理卡下标（PR 表键）
	*/
	void Finalize(const QString& key, const std::shared_ptr<CVABaseDecoder>& decoder, UINT gpuIndex);

	// 逐条 UnInit（锁外执行）并清空
	void ClearAll();

	// 诊断：当前共享条目数
	size_t Size() const;

private:
	mutable std::recursive_mutex m_mtx;
	std::map<QString, std::shared_ptr<CVABaseDecoder>> m_softDecoders;
	UINT m_nextArrivalSeq = 1; // 到达序号发号器（m_mtx 保护），先来后到音频归属依据
};

} // namespace PlayCore
