#include "TimeLineLayer.h"
#include "TimeLineObject.h"
#include "Common/CommonRapidJson.h"

#include <QDebug>
#include "Common/SignalCenter/GSNetSignalCenter.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTimeLineLayer::CTimeLineLayer() 
{
	m_id = QUuid::createUuid();
}

CTimeLineLayer::~CTimeLineLayer()
{
	for (auto& iter : m_hashObjects)
	{
		iter.second.reset();
	}
}

CTimeLineLayer::CTimeLineLayer(const CTimeLineLayer & other)
{
	m_id = other.m_id;
	m_name = other.m_name;
	m_locked = other.m_locked;
	m_hidden = other.m_hidden;
	m_muted = other.m_muted;
	m_bShowPlayProgress = other.m_bShowPlayProgress;
	m_audioMachineId = other.m_audioMachineId;
	m_audioMachineName = other.m_audioMachineName;
	m_audioDeviceKey = other.m_audioDeviceKey;
	m_audioDeviceName = other.m_audioDeviceName;
	m_color = other.m_color;
	m_mapObjects = other.m_mapObjects;

	for (const auto& iter : other.m_hashObjects)
	{
		auto pNode = std::shared_ptr<ITimeLineObject>(dynamic_cast<ITimeLineObject*>(iter.second->Copy()));
		m_hashObjects[pNode->GetId()] = pNode;
	}
}

CTimeLineLayer & CTimeLineLayer::operator=(const CTimeLineLayer & other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);

	m_id = other.m_id; 
	m_name = other.m_name;
	m_locked = other.m_locked;
	m_hidden = other.m_hidden;
	m_muted = other.m_muted;
	m_bShowPlayProgress = other.m_bShowPlayProgress;
	m_audioMachineId = other.m_audioMachineId;
	m_audioMachineName = other.m_audioMachineName;
	m_audioDeviceKey = other.m_audioDeviceKey;
	m_audioDeviceName = other.m_audioDeviceName;
	m_color = other.m_color;
	m_mapObjects = other.m_mapObjects;

	m_hashObjects.clear();
	for (const auto& iter : other.m_hashObjects)
	{
		auto pNode = std::shared_ptr<ITimeLineObject>(dynamic_cast<ITimeLineObject*>(iter.second->Copy()));
		m_hashObjects[pNode->GetId()] = pNode;
	}

	return *this;
}

std::weak_ptr<ITimeLineObject> CTimeLineLayer::GetObject(const std::chrono::milliseconds& ms, bool bCheck)
{
	//查找键值小于等于ms（且键值最大）的object，默认bCheck=true，保证object时长包含ms

	std::shared_lock<std::shared_mutex> lock(m_mtx);
	//返回第一个键值大于ms时间的object
	auto it = m_mapObjects.upper_bound(ms);
	if (it != m_mapObjects.begin()) 
	{
		--it;
		auto iter = m_hashObjects.find(it->second);
		if (iter != m_hashObjects.end())
		{
			if (bCheck)
			{
				if (iter->second->GetStartTime() + iter->second->GetDuration() >= ms)
					return iter->second;
			}
			else
				return iter->second;
		}
	}

	return std::weak_ptr<ITimeLineObject>();
}

void CTimeLineLayer::AddObject(const QUuid& tlId, std::shared_ptr<ITimeLineObject> p)
{
	if (InsertObject(p))
		emit g_pGSNetSignal->signalAddTimelineLayerObject(tlId, m_id, p->GetId());
}

bool CTimeLineLayer::InsertObject(std::shared_ptr<ITimeLineObject> p)
{
	if (!p)
		return false;

	{
		std::unique_lock<std::shared_mutex> lock(m_mtx);
		auto it = m_hashObjects.find(p->GetId());
		if (it != m_hashObjects.end())
			return false;

		m_hashObjects.insert({ p->GetId(), p });
		m_mapObjects.insert({ p->GetStartTime(),p->GetId() });
	}
	return true;
}

void CTimeLineLayer::MoveObject(const QUuid& tlId, const QUuid& id)
{
	{
		std::unique_lock<std::shared_mutex> lock(m_mtx);

		auto it = m_hashObjects.find(id);
		if (it == m_hashObjects.end())
			return;

		auto iter = std::find_if(m_mapObjects.begin(), m_mapObjects.end(),
			[&id](const auto& pair) {
			return pair.second == id;
		}
		);

		m_mapObjects.erase(iter);
		m_mapObjects.insert({ it->second->GetStartTime(),it->second->GetId() });
	}
	emit g_pGSNetSignal->signalMoveTimelineLayerObject(tlId, m_id, id);
	emit g_pGSNetSignal->signalMoveSameLayerTimelineLayerObject(tlId, m_id, id);
}

void CTimeLineLayer::DeleteObject(const QUuid& tlId, const QUuid& id)
{
	std::shared_ptr<ITimeLineObject> removedObject;
	if (ExtractObject(id, removedObject))
		emit g_pGSNetSignal->signalDelTimelineLayerObject(tlId, m_id, id);
}

bool CTimeLineLayer::ExtractObject(const QUuid& id, std::shared_ptr<ITimeLineObject>& outObject)
{
	{
		std::unique_lock<std::shared_mutex> lock(m_mtx);
		auto it = m_hashObjects.find(id);
		if (it == m_hashObjects.end())
			return false;

		auto iter = std::find_if(m_mapObjects.begin(), m_mapObjects.end(),
			[&id](const auto& pair) {
			return pair.second == id;
		}
		);

		outObject = it->second;
		if (iter != m_mapObjects.end())
			m_mapObjects.erase(iter);
		m_hashObjects.erase(it);
	}
	return true;
}

void CTimeLineLayer::ClearObjects()
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashObjects.clear();
	m_mapObjects.clear();
}

int CTimeLineLayer::GetObjectCount()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_hashObjects.size();
}

bool CTimeLineLayer::HasTimelineObjects()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return !m_hashObjects.empty();
}

bool CTimeLineLayer::IsObjectExist(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_hashObjects.find(id) != m_hashObjects.end();
}

std::vector<QUuid> CTimeLineLayer::GetAllObjectIDs()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	std::vector<QUuid> res;
	std::for_each(m_mapObjects.begin(), m_mapObjects.end(), [&](const auto& it) {res.push_back(it.second); });
	return res;
}

std::weak_ptr<ITimeLineObject> CTimeLineLayer::GetObject(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto it = m_hashObjects.find(id);
	if (it != m_hashObjects.end())
		return it->second;

	return std::weak_ptr<ITimeLineObject>();
}

std::chrono::milliseconds CTimeLineLayer::GetMaxTime()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto it = m_mapObjects.rbegin();
	if (it != m_mapObjects.rend())
	{
		auto iter = m_hashObjects.find(it->second);
		if (iter != m_hashObjects.end())
		{
			return iter->second->GetStartTime() + iter->second->GetDuration();
		}
	}

	return std::chrono::milliseconds();
}

QString CTimeLineLayer::GetName()
{
	return m_name;
}

void CTimeLineLayer::SetName(const QString& name)
{
	m_name = name;
}

QUuid CTimeLineLayer::GetId()
{
	return m_id;
}

bool CTimeLineLayer::IsLock()
{
	return m_locked;
}

void CTimeLineLayer::SetLock(bool b)
{
	m_locked = b;
}

bool CTimeLineLayer::IsHide()
{
	return m_hidden;
}

void CTimeLineLayer::SetHide(bool b)
{
	m_hidden = b;
}

bool CTimeLineLayer::IsMute()
{
	return m_muted;
}

void CTimeLineLayer::SetMute(bool b)
{
	m_muted = b;
}

QColor CTimeLineLayer::GetColor()
{
	return m_color;
}

void CTimeLineLayer::SetColor(const QColor& v)
{
	m_color = v;
}

bool CTimeLineLayer::IsShowPlayProgress()
{
	return m_bShowPlayProgress;
}

void CTimeLineLayer::SetShowPlayProgress(bool b)
{
	m_bShowPlayProgress = b;
}

QUuid CTimeLineLayer::GetAudioMachineId()
{
	return m_audioMachineId;
}

QString CTimeLineLayer::GetAudioMachineName()
{
	return m_audioMachineName;
}

void CTimeLineLayer::SetAudioMachine(const QUuid& machineId, const QString& machineName)
{
	m_audioMachineId = machineId;
	m_audioMachineName = machineName;
}

QString CTimeLineLayer::GetAudioDeviceKey()
{
	return m_audioDeviceKey;
}

QString CTimeLineLayer::GetAudioDeviceName()
{
	return m_audioDeviceName;
}

void CTimeLineLayer::SetAudioDevice(const QString& deviceKey, const QString& deviceName)
{
	m_audioDeviceKey = deviceKey;
	m_audioDeviceName = deviceName;
}

void CTimeLineLayer::ClearAudioDevice()
{
	m_audioDeviceKey.clear();
	m_audioDeviceName.clear();
}

#define TimeLineLayer_Name_Json "TimeLineLayer_Name"
#define TimeLineLayer_Lock_Json "TimeLineLayer_Lock"
#define TimeLineLayer_Hide_Json "TimeLineLayer_Hide"
#define TimeLineLayer_Mute_Json "TimeLineLayer_Mute"
#define TimeLineLayer_Id_Json "TimeLineLayer_Id"
#define TimeLineLayer_Color_Json "TimeLineLayer_Color"
#define TimeLineLayer_Objects_Json "TimeLineLayer_Objects"
#define TimeLineLayer_ShowPlayProgress_Json "TimeLineLayer_ShowPlayProgress"
#define TimeLineLayer_AudioMachineId_Json "TimeLineLayer_AudioMachineId"
#define TimeLineLayer_AudioMachineName_Json "TimeLineLayer_AudioMachineName"
#define TimeLineLayer_AudioDeviceKey_Json "TimeLineLayer_AudioDeviceKey"
#define TimeLineLayer_AudioDeviceName_Json "TimeLineLayer_AudioDeviceName"

bool CTimeLineLayer::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
	{
		return false;
	}
	if (!root.HasMember(TimeLineLayer_Name_Json) || !root[TimeLineLayer_Name_Json].IsString())
	{
		return false;
	}
	m_name = QString::fromUtf8(root[TimeLineLayer_Name_Json].GetString());

	if (!root.HasMember(TimeLineLayer_Lock_Json) || !root[TimeLineLayer_Lock_Json].IsBool())
	{
		return false;
	}
	m_locked = root[TimeLineLayer_Lock_Json].GetBool();

	if (!root.HasMember(TimeLineLayer_Hide_Json) || !root[TimeLineLayer_Hide_Json].IsBool())
	{
		return false;
	}
	m_hidden = root[TimeLineLayer_Hide_Json].GetBool();

	if (!root.HasMember(TimeLineLayer_Mute_Json) || !root[TimeLineLayer_Mute_Json].IsBool())
	{
		return false;
	}
	m_muted = root[TimeLineLayer_Mute_Json].GetBool();

	if (!root.HasMember(TimeLineLayer_Id_Json) || !root[TimeLineLayer_Id_Json].IsString())
	{
		return false;
	}
	m_id = QUuid::fromString(QString::fromUtf8(root[TimeLineLayer_Id_Json].GetString()));

	if (!root.HasMember(TimeLineLayer_Color_Json) || !root[TimeLineLayer_Color_Json].IsArray())
	{
		return false;
	}
	const auto& color = root[TimeLineLayer_Color_Json];
	if (color.Size() != 3 || !color[0].IsInt() || !color[1].IsInt() || !color[2].IsInt())
	{
		return false;
	}
	int r = color[0].GetInt();
	int g = color[1].GetInt();
	int b = color[2].GetInt();
	m_color = QColor(r, g, b);

	if (!root.HasMember(TimeLineLayer_Objects_Json) || !root[TimeLineLayer_Objects_Json].IsArray())
	{
		return false;
	}
	const auto& jsonObjects = root[TimeLineLayer_Objects_Json];

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashObjects.clear();
	m_mapObjects.clear();
	for (const auto& jsonObject : jsonObjects.GetArray())
	{
		auto p= std::shared_ptr<ITimeLineObject>(new CTimeLineObject());
		if (p->LoadDataFromRapidJson(jsonObject))
		{
			m_hashObjects.insert({ p->GetId(),p });
			m_mapObjects.insert({ p->GetStartTime(),p->GetId() });
		}
		else
		{
			return false;
		}
	}

	if (root.HasMember(TimeLineLayer_ShowPlayProgress_Json) && root[TimeLineLayer_ShowPlayProgress_Json].IsBool())
		m_bShowPlayProgress = root[TimeLineLayer_ShowPlayProgress_Json].GetBool();
	else
		m_bShowPlayProgress = true;

	if (root.HasMember(TimeLineLayer_AudioMachineId_Json) && root[TimeLineLayer_AudioMachineId_Json].IsString())
		m_audioMachineId = QUuid(QString::fromUtf8(root[TimeLineLayer_AudioMachineId_Json].GetString()));
	else
		m_audioMachineId = QUuid();

	if (root.HasMember(TimeLineLayer_AudioMachineName_Json) && root[TimeLineLayer_AudioMachineName_Json].IsString())
		m_audioMachineName = QString::fromUtf8(root[TimeLineLayer_AudioMachineName_Json].GetString());
	else
		m_audioMachineName.clear();

	if (root.HasMember(TimeLineLayer_AudioDeviceKey_Json) && root[TimeLineLayer_AudioDeviceKey_Json].IsString())
		m_audioDeviceKey = QString::fromUtf8(root[TimeLineLayer_AudioDeviceKey_Json].GetString());
	else
		m_audioDeviceKey.clear();

	if (root.HasMember(TimeLineLayer_AudioDeviceName_Json) && root[TimeLineLayer_AudioDeviceName_Json].IsString())
		m_audioDeviceName = QString::fromUtf8(root[TimeLineLayer_AudioDeviceName_Json].GetString());
	else
		m_audioDeviceName.clear();

	return true;
}

void CTimeLineLayer::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Name_Json), rapidjson::Value(m_name.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Lock_Json), m_locked, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Hide_Json), m_hidden, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Mute_Json), m_muted, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_ShowPlayProgress_Json), m_bShowPlayProgress, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_AudioMachineId_Json),
		rapidjson::Value(m_audioMachineId.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_AudioMachineName_Json),
		rapidjson::Value(m_audioMachineName.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_AudioDeviceKey_Json),
		rapidjson::Value(m_audioDeviceKey.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_AudioDeviceName_Json),
		rapidjson::Value(m_audioDeviceName.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Id_Json), rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);

	rapidjson::Value color(rapidjson::kArrayType);
	color.PushBack(m_color.red(), allocator);
	color.PushBack(m_color.green(), allocator);
	color.PushBack(m_color.blue(), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Color_Json), color, allocator);

	rapidjson::Value jsonObjects(rapidjson::kArrayType);
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (const auto& it : m_hashObjects)
	{
		rapidjson::Value object(rapidjson::kObjectType);
		it.second->SaveDataToRapidJson(object, allocator);
		jsonObjects.PushBack(object, allocator);
	}
	root.AddMember(rapidjson::StringRef(TimeLineLayer_Objects_Json), jsonObjects, allocator);
}
