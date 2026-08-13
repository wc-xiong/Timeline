#pragma once
#include "AudioRenderGlobal.h"

#include <mutex>
#include <atomic>
#include <set>
#include <map>
#include "Common/Common/IAudioCtrl.h"
#include "Common/Common/OneNLink.h"
#include <QObject>
#include <shared_mutex>

#include <thread>

class CAudioRender;
class QTimer;
class CTimer;

class AUDIORENDER_EXPORT CAudioCtrl : public QObject, public IAudioCtrl
{
	Q_OBJECT
public:
	CAudioCtrl(UINT idxDevice, AudioCallback pCallbacker);
	virtual ~CAudioCtrl();

	std::pair<float, float> GetVolumn() { return std::pair<float, float>(m_audioDB.leftAudioDB, m_audioDB.rightAudioDB); }

public:
	// SDL音频播放回调
	void	SdlCallback(SAudioPlayerProp audioPty, uint8_t *stream, int len);
	HRESULT	AddAudioPlayer(SAudioPlayerProp& audioPty, const SPlayerId& playerId) override;
	HRESULT	AddAudioPlayerResolved(SAudioPlayerProp& audioPty, const SPlayerId& playerId) override;
	HRESULT RemoveAudioPlayer(const SPlayerId& playerId) override;
	HRESULT RemoveAllAudioPlayer() override;

	void OnLayerSpeakerChanged(const QUuid& layerId) override;
	void OnLayerMachineChanged(const QUuid& layerId) override;

signals:
	void signalAudioDBUpdate(const SAudioDB &sdb);	// 音量音幅控件
	void signalAudioCtrlShowState(bool bShow);		// 是否显示音频控件

public slots:
	void slotAudioSpeakerChange(const QString& name);

	void slotCleanFinishedPlayers();	// 清理已经播放完了的延时的播放器（当前使用单独线程释放，其他地方不能调用此函数）
	void slotAudioCaptureAuditioning(bool on);

	void slotUpdateAudioValue();

protected:

	void OnAudioDeviceUpdate();			// 音频设备更新了 (热插拔等)

	// 清理不再使用的播放器 -> m_playersCached
	void CleanUnusedPlayers();
	// 音量(音幅)计算
	void Spectrum(const SAudioPlayerProp &audioPty, uint8_t *stream, int len);

	QString FilterSpeaker(const QString& speaker);
	QString ResolveLayerSpeaker(const SPlayerId& playerId) const;
	bool IsPlayerLocal(const SPlayerId& playerId) const;
	SAudioDB CalculateAudioDB(uint8_t* stream, ULONG len, int channels);
	HRESULT AddAudioPlayerInternal(SAudioPlayerProp& pty, const SPlayerId& playerId, bool bRouteResolved);

private:
	UINT m_idxDevice = 0;
	std::shared_timed_mutex m_mutex;
	std::atomic<bool> m_forceMute = false;
	QString m_defaultSpeaker; // 全局默认
	std::map<SAudioPlayerProp, std::shared_ptr<CAudioRender>> m_players; // 所有的播放器, 属性作为 id，相同音频属性的为同一个播放器

	// 音频属性 -> 播放器标识集合
	std::map<SAudioPlayerProp, std::set<SPlayerId>> m_ptyIdLink;
	// 记录播放器是否为本地
	std::map<SPlayerId, bool> m_playerIsLocalMap;

	int m_lenth;
	SAudioDB m_audioDB;
	SAudioDB m_lastUpdateAudioDB;
	std::map<SAudioPlayerProp, SAudioDB> m_mapUpdateAudioValue;
	std::shared_timed_mutex m_mtxUpdateAudioValue;

	AudioCallback m_pAudioCallbacker;									// 音频播放回调
	std::recursive_mutex m_mtxCache;
	std::set<std::shared_ptr<CAudioRender>> m_playersCached;			// 还在延时播放的

	QTimer* m_pTimerUpdateAudioValue = nullptr;

	CTimer* m_pTrdCleanPlayer = nullptr;
};
