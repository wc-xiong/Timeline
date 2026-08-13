#include "AudioCtrl.h"
#include "AudioMix.h"
#include <algorithm>
#include "Common/CommonFunctionApi.h"
#include "AudioRender.h"
#include <memory>
#include "Common/CommonNonPlatform.h"
#include "Common/Timer.h"
#include <QDebug>
#include <QTimer>
#include "GSMedia/TimeServer/TimeServerExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/GlobalModel/GlobalModel.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define DEFINE_LEN  (4096*5)

namespace
{
	std::shared_ptr<ITimeLineLayer> FindLayerById(const QUuid& layerId)
	{
		if (!g_pTimeLineModel || layerId.isNull())
			return nullptr;
		return g_pTimeLineModel->GetLayer(layerId).lock();
	}

	std::shared_ptr<ITimeLineLayer> FindLayerByMediaId(const QUuid& mediaId)
	{
		if (!g_pTimeLineModel || mediaId.isNull())
			return nullptr;
		for (const auto& layerId : g_pTimeLineModel->GetAllLayerIDs())
		{
			auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
			if (pLayer && pLayer->IsObjectExist(mediaId))
				return pLayer;
		}
		return nullptr;
	}

	std::shared_ptr<ITimeLineLayer> FindLayerForPlayer(const SPlayerId& playerId)
	{
		if (!playerId.layerId.isNull())
		{
			if (auto pLayer = FindLayerById(playerId.layerId))
				return pLayer;
		}
		return FindLayerByMediaId(playerId.mediaId);
	}

	QString DeviceKeyToSpeaker(const QString& deviceKey)
	{
		if (deviceKey.isEmpty() || deviceKey == QLatin1String("@default"))
			return QString();
		return deviceKey;
	}

	void FreeAudioData(std::vector<SAudioPlayData>& audioData)
	{
		for (auto& data : audioData)
			SafeDeleteArray(data.pAudioData);
		audioData.clear();
	}

}


CAudioCtrl::CAudioCtrl(UINT idxDevice, AudioCallback pCallbacker)
{
	m_idxDevice = idxDevice;
	m_pAudioCallbacker = pCallbacker;
	m_lenth = 0;
	m_audioDB.SetDefault();

	SDL_InitSubSystem(SDL_INIT_AUDIO);

	m_pTimerUpdateAudioValue = new QTimer(this);
	connect(m_pTimerUpdateAudioValue, &QTimer::timeout, this, &CAudioCtrl::slotUpdateAudioValue);
	m_pTimerUpdateAudioValue->start(200);

	m_pTrdCleanPlayer = new CTimer(std::bind(&CAudioCtrl::slotCleanFinishedPlayers, this), 1000.0f);
	m_pTrdCleanPlayer->Start();
}

CAudioCtrl::~CAudioCtrl()
{
	RemoveAllAudioPlayer();

	m_pTrdCleanPlayer->Stop();
	delete m_pTrdCleanPlayer;
}

void CAudioCtrl::SdlCallback(SAudioPlayerProp audioPty, uint8_t *stream, int len)
{
	//qDebug() << "AudioCallback: " << audioPty.GetAudioPtyString() << len;

	SDL_memset(stream, 0, len);

	// 非本地的也需要拉去音频数据, 否则切换时会有一段时间的静音
	std::vector<SPlayerId> idsLocal;
	std::vector<SPlayerId> idsRemote;
	{
		std::shared_lock<std::shared_timed_mutex> locker(m_mutex);
		auto iter = m_ptyIdLink.find(audioPty);
		if (iter == m_ptyIdLink.end())
			return;
		for (const auto& id : iter->second)
		{
			auto localIt = m_playerIsLocalMap.find(id);
			const bool bLocal = (localIt == m_playerIsLocalMap.end()) ? true : localIt->second;
			if (bLocal)
				idsLocal.push_back(id);
			else
				idsRemote.push_back(id);
		}
	}

	std::vector<SAudioPlayData> localAudioData;
	std::vector<SAudioPlayData> remoteAudioData;
	if (m_pAudioCallbacker)
	{
		if (!idsLocal.empty())
			m_pAudioCallbacker(idsLocal, &localAudioData);
		if (!idsRemote.empty())
			m_pAudioCallbacker(idsRemote, &remoteAudioData);
	}

	bool bMixSuccess = MixAudio(stream, len, localAudioData, audioPty);
	FreeAudioData(localAudioData);
	FreeAudioData(remoteAudioData);

	if (m_forceMute)
		SDL_memset(stream, 0, len);

	if (bMixSuccess)
		Spectrum(audioPty, stream, len);
}

HRESULT CAudioCtrl::AddAudioPlayer(SAudioPlayerProp& pty, const SPlayerId& playerId)
{
	pty.speaker = ResolveLayerSpeaker(playerId);
	return AddAudioPlayerInternal(pty, playerId, false);
}

HRESULT CAudioCtrl::AddAudioPlayerResolved(SAudioPlayerProp& pty, const SPlayerId& playerId)
{
	return AddAudioPlayerInternal(pty, playerId, true);
}

QString CAudioCtrl::ResolveLayerSpeaker(const SPlayerId& playerId) const
{
	auto pLayer = FindLayerForPlayer(playerId);
	if (!pLayer)
		return QString();

	return DeviceKeyToSpeaker(pLayer->GetAudioDeviceKey());
}

bool CAudioCtrl::IsPlayerLocal(const SPlayerId& playerId) const
{
	// 查不到图层：不混音，避免多机误出声
	auto pLayer = FindLayerForPlayer(playerId);
	if (!pLayer)
		return false; 

	const QUuid machineId = pLayer->GetAudioMachineId();
	// 未配置机器：视为本地
	if (machineId.isNull())
		return true; 

	return machineId == g_pGlobalModel->GetMachineId();
}

void CAudioCtrl::OnLayerSpeakerChanged(const QUuid& layerId)
{
	if (layerId.isNull() || !g_pTimeLineModel)
		return;

	auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
	if (!pLayer)
		return;

	const QString newSpeaker = DeviceKeyToSpeaker(pLayer->GetAudioDeviceKey());
	std::vector<std::pair<SPlayerId, SAudioPlayerProp>> idPtyChanged;
	{
		std::shared_lock<std::shared_timed_mutex> locker(m_mutex);
		for (const auto& link : m_ptyIdLink)
		{
			for (const SPlayerId& playerId : link.second)
			{
				if (playerId.layerId == layerId)
					idPtyChanged.emplace_back(playerId, link.first);
			}
		}
	}

	for (auto& idPty : idPtyChanged)
	{
		if (idPty.second.speaker == newSpeaker)
		{
			std::unique_lock<std::shared_timed_mutex> locker(m_mutex);
			m_playerIsLocalMap[idPty.first] = IsPlayerLocal(idPty.first);
			continue;
		}

		RemoveAudioPlayer(idPty.first);
		idPty.second.speaker = newSpeaker;
		AddAudioPlayer(idPty.second, idPty.first);
	}

	CleanUnusedPlayers();
}

void CAudioCtrl::OnLayerMachineChanged(const QUuid& layerId)
{
	if (layerId.isNull() || !g_pTimeLineModel)
		return;
	auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
	if (!pLayer)
		return;

	std::vector<SPlayerId> affectedIds;
	{
		std::shared_lock<std::shared_timed_mutex> locker(m_mutex);
		for (const auto& link : m_ptyIdLink)
		{
			for (const SPlayerId& playerId : link.second)
			{
				if (playerId.layerId == layerId)
					affectedIds.push_back(playerId);
			}
		}
	}

	std::vector<std::pair<SPlayerId, bool>> updates;
	updates.reserve(affectedIds.size());
	for (const SPlayerId& playerId : affectedIds)
		updates.emplace_back(playerId, IsPlayerLocal(playerId));

	std::unique_lock<std::shared_timed_mutex> locker(m_mutex);
	for (const auto& item : updates)
		m_playerIsLocalMap[item.first] = item.second;
}

HRESULT CAudioCtrl::AddAudioPlayerInternal(SAudioPlayerProp& pty, const SPlayerId& playerId, bool bRouteResolved)
{
	bool bPtyEmpty = (pty.sampleRate <= 0 || pty.channels <= 0);

	std::shared_ptr<CAudioRender> player = nullptr;
	bool bBeforeAudioEmpty = m_players.empty();
	if (!bPtyEmpty)
	{
		auto itPlayer = m_players.find(pty);
		if (itPlayer == m_players.end())
		{
			player = std::make_shared<CAudioRender>(pty, this);
			const QString speakerName = bRouteResolved ? pty.speaker : FilterSpeaker(pty.speaker);
			player->Init(speakerName.toStdString());
			m_players[pty] = player;
		}
		else
		{
			player = itPlayer->second;
		}
	}

	bool bLocal = IsPlayerLocal(playerId);
	{
		std::unique_lock<std::shared_timed_mutex> locker(m_mutex);
		m_playerIsLocalMap[playerId] = bLocal;
		if (!bPtyEmpty)
		{
			auto iter = m_ptyIdLink.find(pty);
			if (iter != m_ptyIdLink.end())
			{
				iter->second.insert(playerId);
			}
			else
			{
				std::set<SPlayerId> set;
				set.insert(playerId);
				m_ptyIdLink[pty] = set;
			}
		}
	}

	if (bBeforeAudioEmpty)
	{
		m_audioDB.SetDefault();
		emit signalAudioCtrlShowState(true);
	}

	if (player)
		player->Start();

	return S_OK;
}

HRESULT CAudioCtrl::RemoveAudioPlayer(const SPlayerId& playerId)
{
	SAudioPlayerProp removePty;
	bool bRemove = false;
	bool bLocal = true;
	{
		std::unique_lock<std::shared_timed_mutex> locker(m_mutex);

		auto localIt = m_playerIsLocalMap.find(playerId);
		if (localIt != m_playerIsLocalMap.end())
		{
			bLocal = localIt->second;
			m_playerIsLocalMap.erase(localIt);
		}

		for (auto iter = m_ptyIdLink.begin(); iter != m_ptyIdLink.end(); ++iter)
		{
			auto resIter = iter->second.find(playerId);
			if (resIter != iter->second.end())
			{
				iter->second.erase(resIter);
				if (iter->second.empty())
				{
					bRemove = true;
					removePty = iter->first;
				}
				break;
			}
		}
	}

	if (bRemove)
	{
		auto iter = m_players.find(removePty);
		if (iter != m_players.end() && iter->second)
		{
			auto player = iter->second;
			player->Close();
			{
				std::lock_guard<std::recursive_mutex> lock(m_mtxCache);
				m_playersCached.insert(player);
			}
			m_players.erase(iter);
		}
	}

	if (m_players.empty())
	{
		std::unique_lock<std::shared_timed_mutex> LockUpdate(m_mtxUpdateAudioValue);
		m_mapUpdateAudioValue.clear();
	}

	// 仅本地移除时清理未使用 SDL，避免远端移除关掉仍被本地占用的设备
	if (bLocal)
		CleanUnusedPlayers();

	return S_OK;
}

HRESULT CAudioCtrl::RemoveAllAudioPlayer()
{
	{
		std::unique_lock<std::shared_timed_mutex> locker(m_mutex);
		m_ptyIdLink.clear();
		m_playerIsLocalMap.clear();
	}

	for (const auto &propPlayer : m_players)
	{
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxCache);
			m_playersCached.insert(propPlayer.second);
		}
		propPlayer.second->Close();
	}

	m_players.clear();

	return S_OK;
}

void CAudioCtrl::slotAudioSpeakerChange(const QString& name)
{
	qDebug() << "slotAudioSpeakerChange " << name;

	m_defaultSpeaker = name;

	for (const auto &ptyPlayer : m_players)
	{
		if (ptyPlayer.first.speaker.isEmpty())
		{
			ptyPlayer.second->Init(m_defaultSpeaker.toStdString());
			ptyPlayer.second->Start();
		}
	}

	// 清理未使用的播放器
	CleanUnusedPlayers();
}

void CAudioCtrl::slotAudioCaptureAuditioning(bool on)
{
	m_forceMute = on;
}

void CAudioCtrl::slotUpdateAudioValue()
{
	std::chrono::microseconds Ts;
	TimeServer_GetCurTime(Ts);
	auto curTs = Ts.count();

	SAudioDB dbValue;
	for (auto it : m_players)
	{
		std::shared_lock<std::shared_timed_mutex> LockUpdate(m_mtxUpdateAudioValue);
		auto dbItem = m_mapUpdateAudioValue.find(it.first);
		if (dbItem == m_mapUpdateAudioValue.end())
			continue;
		if (curTs - dbItem->second.curPts > 200)
			continue;
		dbValue.leftAudioDB = std::max(dbValue.leftAudioDB, dbItem->second.leftAudioDB);
		dbValue.rightAudioDB = std::max(dbValue.rightAudioDB, dbItem->second.rightAudioDB);
	}

	if (!(m_lastUpdateAudioDB == dbValue))
	{
		emit signalAudioDBUpdate(dbValue);
		m_lastUpdateAudioDB = dbValue;
	}
}

void CAudioCtrl::OnAudioDeviceUpdate()
{
	for (auto &ptyPlayer : m_players)
	{
		// 尝试重新打开设备
		if (ptyPlayer.second->IsStopped())
		{
			//ptyPlayer.second->Close();
			ptyPlayer.second->Init(FilterSpeaker(ptyPlayer.first.speaker).toStdString());
			ptyPlayer.second->Start();
		}
	}
}

void CAudioCtrl::CleanUnusedPlayers()
{
	std::vector<SAudioPlayerProp> ptys;
	for (const auto &ptyPlayer : m_players)
	{
		std::shared_lock<std::shared_timed_mutex> locker(m_mutex);
		auto iter = m_ptyIdLink.find(ptyPlayer.first);
		if (iter == m_ptyIdLink.end() || iter->second.empty())
		{
			ptys.push_back(ptyPlayer.first);
		}
	}

	for (const auto &pty : ptys)
	{
		auto iter = m_players.find(pty);
		if (iter == m_players.end() || !iter->second)
			continue;

		auto player = iter->second;
		m_players.erase(iter);
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxCache);
			m_playersCached.insert(player);
		}
		player->Close();
	}
}

void CAudioCtrl::Spectrum(const SAudioPlayerProp & audioPty, uint8_t * stream, int len)
{
	SAudioDB audioDB = CalculateAudioDB(stream, (ULONG)len, (int)audioPty.channels);
	float p = float(len) / float(DEFINE_LEN);
	audioDB.leftAudioDB *= p;
	audioDB.rightAudioDB *= p;

	if (m_lenth < DEFINE_LEN)
	{
		m_audioDB.leftAudioDB += audioDB.leftAudioDB;
		m_audioDB.rightAudioDB += audioDB.rightAudioDB;
		m_lenth += len;
	}
	else
	{
		std::chrono::microseconds Ts;
		TimeServer_GetCurTime(Ts);
		m_audioDB.curPts = Ts.count();

		{
			std::unique_lock<std::shared_timed_mutex> LockUpdate(m_mtxUpdateAudioValue);
			m_mapUpdateAudioValue[audioPty] = m_audioDB;
		}

		m_lenth = len;
		m_audioDB.leftAudioDB = audioDB.leftAudioDB;
		m_audioDB.rightAudioDB = audioDB.rightAudioDB;
	}
}

QString CAudioCtrl::FilterSpeaker(const QString& speaker)
{
	if (speaker.isEmpty())
		return m_defaultSpeaker;

	return speaker;
}

void CAudioCtrl::slotCleanFinishedPlayers()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxCache);
	for (auto it = m_playersCached.begin(); it != m_playersCached.end();)
	{
		if ((*it)->IsFinished())
		{
			it = m_playersCached.erase(it);
		}
		else
		{
			it++;
		}
	}
}

SAudioDB CAudioCtrl::CalculateAudioDB(uint8_t * stream, ULONG lenth, int channels)
{
	SAudioDB audioDB;
	if (lenth)
	{
		//左声道总振幅值
		int sumL = 0;
		//右声道总振幅值
		int sumR = 0;
		//左声道平均振幅值
		float averL = 0.0f;
		//右声道平均振幅值
		float averR = 0.0f;

		if (channels == 2)
		{
			for (int i = 0; i < int(lenth); i += 4)
			{
				sumL += abs(*(short*)(&stream[i]));
				sumR += abs(*(short*)(&stream[i + 2]));
			}
			averL = (float)sumL / DOUBLE_BYTE / lenth * 4;
			averR = (float)sumR / DOUBLE_BYTE / lenth * 4;
			audioDB.leftAudioDB = CalcuDB(averL);
			audioDB.rightAudioDB = CalcuDB(averR);
		}
		else if (channels == 1)
		{
			for (int i = 0; i < int(lenth); i += 2)
			{
				sumL += abs(*(short*)(&stream[i]));
				sumR += abs(*(short*)(&stream[i]));
			}
			averL = (float)sumL / DOUBLE_BYTE / lenth * 4;
			averR = averR;
			audioDB.leftAudioDB = CalcuDB(averL);
			audioDB.rightAudioDB = audioDB.leftAudioDB;
		}
		audioDB.leftAudioDB = std::max(audioDB.leftAudioDB, MIN_VOLUME);
		audioDB.rightAudioDB = std::max(audioDB.rightAudioDB, MIN_VOLUME);
	}
	return audioDB;
}
