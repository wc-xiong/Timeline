#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <map>
#include <unordered_map>
#include <shared_mutex>


class CTimeLineLayer : public ITimeLineLayer
{
	INIT_ICOPYINTERFACE(CTimeLineLayer)
public:
	CTimeLineLayer();
	virtual ~CTimeLineLayer();
	CTimeLineLayer(const CTimeLineLayer &other);
	CTimeLineLayer& operator=(const CTimeLineLayer &other);

	virtual QUuid GetId()override;

	virtual std::weak_ptr<ITimeLineObject> GetObject(const std::chrono::milliseconds& ms, bool bCheck)override;

	virtual void AddObject(const QUuid& tlId, std::shared_ptr<ITimeLineObject> p)override;
	virtual void MoveObject(const QUuid& tlId, const QUuid& id)override;
	virtual void DeleteObject(const QUuid& tlId, const QUuid& id)override;
	virtual bool InsertObject(std::shared_ptr<ITimeLineObject> p)override;
	virtual bool ExtractObject(const QUuid& id, std::shared_ptr<ITimeLineObject>& outObject)override;
	virtual void ClearObjects()override;
	virtual int GetObjectCount()override;
	virtual bool HasTimelineObjects()override;
	virtual bool IsObjectExist(const QUuid& id)override;
	virtual std::vector<QUuid> GetAllObjectIDs()override;
	virtual std::weak_ptr<ITimeLineObject> GetObject(const QUuid& id)override;

	virtual std::chrono::milliseconds GetMaxTime()override;

	virtual QString GetName()override;
	virtual void SetName(const QString& name)override;

	virtual bool IsLock()override;
	virtual void SetLock(bool b)override;

	virtual bool IsHide()override;
	virtual void SetHide(bool b)override;

	virtual bool IsMute()override;
	virtual void SetMute(bool b)override;

	virtual QColor GetColor()override;
	virtual void SetColor(const QColor& v)override;

	virtual bool IsShowPlayProgress()override;
	virtual void SetShowPlayProgress(bool b)override;
	virtual QUuid GetAudioMachineId()override;
	virtual QString GetAudioMachineName()override;
	virtual void SetAudioMachine(const QUuid& machineId, const QString& machineName)override;
	virtual QString GetAudioDeviceKey()override;
	virtual QString GetAudioDeviceName()override;
	virtual void SetAudioDevice(const QString& deviceKey, const QString& deviceName)override;
	virtual void ClearAudioDevice()override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root)override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)override;

private:
	std::shared_mutex	m_mtx;
	std::unordered_map<QUuid, std::shared_ptr<ITimeLineObject>, SUuidHashFunc>	m_hashObjects;	//以id为键存储
	std::map<std::chrono::milliseconds, QUuid>m_mapObjects;	//object开始时间为键存储

	QString m_name;
	bool m_locked = false;
	bool m_hidden = false;
	bool m_muted = false;
	bool m_bShowPlayProgress = false;
	QUuid m_audioMachineId;
	QString m_audioMachineName;
	QString m_audioDeviceKey;
	QString m_audioDeviceName;
	QUuid m_id;

	QColor m_color = QColor(61, 139, 255);
};