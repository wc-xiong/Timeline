#include "TimeLineObject.h"
#include "ConvertHelper.h"

#include "Common/MediaPropertyKeyframeIds.h"
#include "Common/CommonRapidJson.h"
#include <QDebug>
#include <set>
#include <string>
#include "GSMedia/RenderCommon/SimpleMath.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
	void SaveHandleToJson(const SPropertyKeyframeHandle& handle, rapidjson::Value& outObj,
		rapidjson::Document::AllocatorType& allocator)
	{
		outObj.SetObject();
		outObj.AddMember("timeOffsetMs", handle.timeOffsetMs, allocator);
		outObj.AddMember("valueOffset", handle.valueOffset, allocator);
		outObj.AddMember("bOpen", handle.bOpen, allocator);
	}

	bool LoadHandleFromJson(const rapidjson::Value& obj, SPropertyKeyframeHandle& handle)
	{
		if (!obj.IsObject())
			return false;
		if (obj.HasMember("timeOffsetMs") && obj["timeOffsetMs"].IsNumber())
			handle.timeOffsetMs = obj["timeOffsetMs"].GetFloat();
		if (obj.HasMember("valueOffset") && obj["valueOffset"].IsNumber())
			handle.valueOffset = obj["valueOffset"].GetFloat();
		if (obj.HasMember("bOpen") && obj["bOpen"].IsBool())
			handle.bOpen = obj["bOpen"].GetBool();
		return true;
	}

	constexpr float kHandleDefaultTimeRatio = 0.33f;
}
inline CTimeLineObjectFx::CTimeLineObjectFx()
{
}

inline CTimeLineObjectFx::CTimeLineObjectFx(int type)
{
	m_type = type;
}

inline CTimeLineObjectFx::~CTimeLineObjectFx()
{
}

inline CTimeLineObjectFx::CTimeLineObjectFx(const CTimeLineObjectFx& other)
{
	m_type = other.m_type;
	m_mapValues = other.m_mapValues;
	m_mapKeyframeNodes = other.m_mapKeyframeNodes;
}

inline CTimeLineObjectFx& CTimeLineObjectFx::operator=(const CTimeLineObjectFx& other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);

	m_type = other.m_type;
	m_mapValues = other.m_mapValues;
	m_mapKeyframeNodes = other.m_mapKeyframeNodes;

	return *this;
}

inline int CTimeLineObjectFx::GetType()
{
	return m_type;
}

inline void CTimeLineObjectFx::AddValue(const std::chrono::milliseconds& ms, const FxData& v)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase && std::holds_alternative<float>(v))
	{
		SPropertyKeyframeNode node;
		node.value = std::get<float>(v);
		m_mapKeyframeNodes[ms] = node;
	}
	else
	{
		m_mapValues[ms] = v;
	}
}

inline void CTimeLineObjectFx::DelereValue(const std::chrono::milliseconds& ms)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
		m_mapKeyframeNodes.erase(ms);
	else
		m_mapValues.erase(ms);
}

inline void CTimeLineObjectFx::ClearValue()
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_mapValues.clear();
	m_mapKeyframeNodes.clear();
}

inline FxData CTimeLineObjectFx::GetValue(const std::chrono::milliseconds& ms)
{
	if (m_type >= PropertyKeyframeTypeBase)
		return EvaluatePropertyKeyframeValue(ms);

	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapValues.find(ms);
	if (iter != m_mapValues.end())
	{
		return iter->second;
	}

	iter = m_mapValues.upper_bound(ms);
	// 首关键帧之前：hold 首关键帧数值（与属性曲线轨对称）
	if (iter == m_mapValues.begin())
	{
		if (iter != m_mapValues.end())
			return iter->second;
		return FxData();
	}

	auto iter2 = iter;
	--iter;
	if (iter2 != m_mapValues.end())
	{
		float t = float((ms - iter->first).count()) / float((iter2->first - iter->first).count());
		if (m_type == ITimeLineObject::FxType_Scale)
		{
			if (std::holds_alternative<DirectX::XMFLOAT3>(iter->second) &&
				std::holds_alternative<DirectX::XMFLOAT3>(iter2->second))
			{
				auto v1 = std::get<DirectX::XMFLOAT3>(iter->second);
				auto v2 = std::get<DirectX::XMFLOAT3>(iter2->second);

				DirectX::XMFLOAT3 res;
				res.x = v1.x + (v2.x - v1.x) * t;
				res.y = v1.y + (v2.y - v1.y) * t;
				res.z = v1.z + (v2.z - v1.z) * t;
				return res;
			}
		}
		else if (m_type == ITimeLineObject::FxType_Size
			|| m_type == ITimeLineObject::FxType_Fov
			|| m_type == ITimeLineObject::FxType_PosX
			|| m_type == ITimeLineObject::FxType_PosY
			|| m_type == ITimeLineObject::FxType_PosZ
			|| m_type == ITimeLineObject::FxType_RotX
			|| m_type == ITimeLineObject::FxType_RotY
			|| m_type == ITimeLineObject::FxType_RotZ)
		{
			if (std::holds_alternative<float>(iter->second) &&
				std::holds_alternative<float>(iter2->second))
			{
				auto v1 = std::get<float>(iter->second);
				auto v2 = std::get<float>(iter2->second);

				return v1 + (v2 - v1) * t;
			}
		}
		else if (m_type == ITimeLineObject::FxType_ObserveTarget)
		{
			// 观察目标阶跃：区间内取左关键帧，不做插值
			if (std::holds_alternative<std::string>(iter->second))
				return iter->second;
		}
		else if (m_type >= PropertyKeyframeTypeBase)
		{
			if (std::holds_alternative<float>(iter->second) &&
				std::holds_alternative<float>(iter2->second))
			{
				auto v1 = std::get<float>(iter->second);
				auto v2 = std::get<float>(iter2->second);
				return v1 + (v2 - v1) * t;
			}
		}
	}
	else
	{
		// 末关键帧之后：hold 末帧
		return iter->second;
	}

	return FxData();
}

std::vector<std::pair<std::chrono::milliseconds, float>> CTimeLineObjectFx::GetFloatKeyframes() const
{
	std::vector<std::pair<std::chrono::milliseconds, float>> result;
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
	{
		result.reserve(m_mapKeyframeNodes.size());
		for (const auto& it : m_mapKeyframeNodes)
			result.emplace_back(it.first, it.second.value);
		return result;
	}
	result.reserve(m_mapValues.size());
	for (const auto& it : m_mapValues)
	{
		if (std::holds_alternative<float>(it.second))
			result.emplace_back(it.first, std::get<float>(it.second));
	}
	return result;
}

bool CTimeLineObjectFx::IsEmpty() const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
		return m_mapKeyframeNodes.empty();
	return m_mapValues.empty();
}

bool CTimeLineObjectFx::HasValueAt(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
		return m_mapKeyframeNodes.find(ms) != m_mapKeyframeNodes.end();
	return m_mapValues.find(ms) != m_mapValues.end();
}

std::vector<std::chrono::milliseconds> CTimeLineObjectFx::GetAllKeyframeTimes() const
{
	std::vector<std::chrono::milliseconds> result;
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
	{
		result.reserve(m_mapKeyframeNodes.size());
		for (const auto& it : m_mapKeyframeNodes)
			result.push_back(it.first);
		return result;
	}
	result.reserve(m_mapValues.size());
	for (const auto& it : m_mapValues)
		result.push_back(it.first);
	return result;
}

float CTimeLineObjectFx::EvaluatePropertyKeyframeValue(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter != m_mapKeyframeNodes.end())
		return iter->second.value;

	iter = m_mapKeyframeNodes.upper_bound(ms);
	// 首关键帧之前：hold 首关键帧数值（与末关键帧之后 hold 末值对称）；空轨返回 0
	if (iter == m_mapKeyframeNodes.begin())
	{
		if (iter == m_mapKeyframeNodes.end())
			return 0.f;
		return iter->second.value;
	}

	auto iter2 = iter;
	--iter;
	if (iter2 == m_mapKeyframeNodes.end())
		return iter->second.value;

	const auto& n0 = iter->second;
	const auto& n1 = iter2->second;
	const float t = float((ms - iter->first).count()) / float((iter2->first - iter->first).count());
	if (IsPropertyKeyframeTrack())
	{
		const int propId = m_type - PropertyKeyframeTypeBase;
		if (IsPropertyKeyframeCurveModeLockedToLine(static_cast<EMediaPropertyId>(propId)))
			return n0.value + (n1.value - n0.value) * t;
	}
	return EvaluatePropertyKeyframeSegmentValue(n0, n1, t);
}

std::vector<std::pair<std::chrono::milliseconds, SPropertyKeyframeNode>> CTimeLineObjectFx::GetKeyframeNodes() const
{
	std::vector<std::pair<std::chrono::milliseconds, SPropertyKeyframeNode>> result;
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	result.reserve(m_mapKeyframeNodes.size());
	for (const auto& it : m_mapKeyframeNodes)
		result.emplace_back(it.first, it.second);
	return result;
}

bool CTimeLineObjectFx::GetKeyframeNodeAt(const std::chrono::milliseconds& ms, SPropertyKeyframeNode& outNode) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter == m_mapKeyframeNodes.end())
		return false;
	outNode = iter->second;
	return true;
}

void CTimeLineObjectFx::SetKeyframeNodeAt(const std::chrono::milliseconds& ms, const SPropertyKeyframeNode& node)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_mapKeyframeNodes[ms] = node;
}

bool CTimeLineObjectFx::MoveKeyframe(const std::chrono::milliseconds& oldMs, const std::chrono::milliseconds& newMs)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(oldMs);
	if (iter == m_mapKeyframeNodes.end() || m_mapKeyframeNodes.find(newMs) != m_mapKeyframeNodes.end())
		return false;
	auto node = iter->second;
	m_mapKeyframeNodes.erase(iter);
	m_mapKeyframeNodes[newMs] = node;
	return true;
}

void CTimeLineObjectFx::SetKeyframeValue(const std::chrono::milliseconds& ms, float value)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_mapKeyframeNodes[ms].value = value;
}

void CTimeLineObjectFx::SetKeyframeCurveMode(const std::chrono::milliseconds& ms, EPropertyKeyframeCurveMode mode)
{
	{
		std::unique_lock<std::shared_mutex> lock(m_mtx);
		auto iter = m_mapKeyframeNodes.find(ms);
		if (iter == m_mapKeyframeNodes.end())
			return;
		iter->second.outCurve = mode;
	}
	CheckKeyframeHandles(ms);
}

EPropertyKeyframeCurveMode CTimeLineObjectFx::GetKeyframeCurveMode(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter == m_mapKeyframeNodes.end())
		return EPropertyKeyframeCurveMode::Line;
	return iter->second.outCurve;
}

void CTimeLineObjectFx::SetKeyframeHandle(const std::chrono::milliseconds& ms, bool bLeft, const SPropertyKeyframeHandle& handle)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter == m_mapKeyframeNodes.end())
		return;
	if (bLeft)
		iter->second.leftHandle = handle;
	else
		iter->second.rightHandle = handle;
}

void CTimeLineObjectFx::CheckKeyframeHandles(const std::chrono::milliseconds& ms)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter == m_mapKeyframeNodes.end())
		return;

	auto& node = iter->second;
	if (node.outCurve == EPropertyKeyframeCurveMode::Line)
	{
		node.leftHandle = {};
		node.rightHandle = {};
		return;
	}

	const bool bFirst = (iter == m_mapKeyframeNodes.begin());
	const bool bLast = (std::next(iter) == m_mapKeyframeNodes.end());
	const bool bBezier = IsPropertyKeyframeBezierCurve(node.outCurve);

	const auto nextIt = std::next(iter);
	if (bLast)
	{
		node.rightHandle = {};
	}
	else
	{
		const float dt = float((nextIt->first - iter->first).count());
		const float dv = nextIt->second.value - node.value;
		if (bBezier && !node.rightHandle.bOpen)
		{
			node.rightHandle.timeOffsetMs = dt * kHandleDefaultTimeRatio;
			node.rightHandle.valueOffset = dv * kHandleDefaultTimeRatio;
			node.rightHandle.bOpen = true;
		}
	}

	if (bFirst)
	{
		node.leftHandle = {};
	}
	else
	{
		auto prevIt = std::prev(iter);
		if (bBezier && !node.leftHandle.bOpen)
		{
			const float dt = float((iter->first - prevIt->first).count());
			const float dv = node.value - prevIt->second.value;
			node.leftHandle.timeOffsetMs = -dt * kHandleDefaultTimeRatio;
			node.leftHandle.valueOffset = -dv * kHandleDefaultTimeRatio;
			node.leftHandle.bOpen = true;
		}
	}

	// 自动曲线仅在中点关键帧镜像双侧手柄；首点仅出手柄，末点仅入手柄
	if (node.outCurve == EPropertyKeyframeCurveMode::AutoBezier)
	{
		if (!bFirst && node.rightHandle.bOpen)
		{
			node.leftHandle.timeOffsetMs = -node.rightHandle.timeOffsetMs;
			node.leftHandle.valueOffset = -node.rightHandle.valueOffset;
			node.leftHandle.bOpen = true;
		}
		else if (!bLast && node.leftHandle.bOpen)
		{
			node.rightHandle.timeOffsetMs = -node.leftHandle.timeOffsetMs;
			node.rightHandle.valueOffset = -node.leftHandle.valueOffset;
			node.rightHandle.bOpen = true;
		}
	}

	if (bFirst)
		node.leftHandle = {};
	if (bLast)
		node.rightHandle = {};
}

bool CTimeLineObjectFx::HasPreKeyframe(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
	{
		if (m_mapKeyframeNodes.empty())
			return false;
		auto iter = m_mapKeyframeNodes.lower_bound(ms);
		return iter != m_mapKeyframeNodes.begin();
	}
	// 经典 Fx（路径相机 Position/Fov 等）
	if (m_mapValues.empty())
		return false;
	auto iter = m_mapValues.lower_bound(ms);
	return iter != m_mapValues.begin();
}

bool CTimeLineObjectFx::HasNextKeyframe(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
	{
		if (m_mapKeyframeNodes.empty())
			return false;
		// 与 UI 导航 upper_bound 语义一致：当前时间之后是否还存在关键帧
		auto iter = m_mapKeyframeNodes.upper_bound(ms);
		return iter != m_mapKeyframeNodes.end();
	}

	// 经典 Fx（路径相机 Position/Fov 等）
	if (m_mapValues.empty())
		return false;
	auto iter = m_mapValues.upper_bound(ms);
	return iter != m_mapValues.end();
}

std::chrono::milliseconds CTimeLineObjectFx::GetPreKeyframeTime(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter == m_mapKeyframeNodes.end() || iter == m_mapKeyframeNodes.begin())
		return std::chrono::milliseconds(-1);
	return std::prev(iter)->first;
}

std::chrono::milliseconds CTimeLineObjectFx::GetNextKeyframeTime(const std::chrono::milliseconds& ms) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_mapKeyframeNodes.find(ms);
	if (iter == m_mapKeyframeNodes.end())
		return std::chrono::milliseconds(-1);
	auto nextIt = std::next(iter);
	if (nextIt == m_mapKeyframeNodes.end())
		return std::chrono::milliseconds(-1);
	return nextIt->first;
}

#define TimeLineObjectFx_type_Json "TimeLineObjectFx_type"
#define TimeLineObjectFx_values_Json "TimeLineObjectFx_values"

bool CTimeLineObjectFx::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
	{
		return false;
	}
	if (!root.HasMember(TimeLineObjectFx_type_Json) || !root[TimeLineObjectFx_type_Json].IsInt())
	{
		return false;
	}
	m_type = root[TimeLineObjectFx_type_Json].GetInt();

	if (!root.HasMember(TimeLineObjectFx_values_Json) || !root[TimeLineObjectFx_values_Json].IsArray())
	{
		return false;
	}
	const auto& jsonValues = root[TimeLineObjectFx_values_Json];

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_mapValues.clear();
	m_mapKeyframeNodes.clear();
	for (const auto& temp : jsonValues.GetArray())
	{
		if (!temp.IsObject() || !temp.HasMember("key") || !temp.HasMember("value") || !temp["key"].IsInt())
		{
			return false;
		}
		int key = temp["key"].GetInt();
		if (m_type == ITimeLineObject::FxType_Scale)
		{
			const auto& v1 = temp["value"];
			if (!v1.IsArray() || v1.Size() != 3 || !v1[0].IsNumber() || !v1[1].IsNumber() || !v1[2].IsNumber())
			{
				return false;
			}
			DirectX::XMFLOAT3 V;
			V.x = v1[0].GetFloat();
			V.y = v1[1].GetFloat();
			V.z = v1[2].GetFloat();

			m_mapValues[std::chrono::milliseconds(key)] = V;
		}
		else if (m_type == ITimeLineObject::FxType_Size
			|| m_type == ITimeLineObject::FxType_Fov
			|| m_type == ITimeLineObject::FxType_PosX
			|| m_type == ITimeLineObject::FxType_PosY
			|| m_type == ITimeLineObject::FxType_PosZ
			|| m_type == ITimeLineObject::FxType_RotX
			|| m_type == ITimeLineObject::FxType_RotY
			|| m_type == ITimeLineObject::FxType_RotZ)
		{
			if (!temp["value"].IsNumber())
			{
				return false;
			}
			m_mapValues[std::chrono::milliseconds(key)] = temp["value"].GetFloat();
		}
		else if (m_type == ITimeLineObject::FxType_ObserveTarget)
		{
			if (!temp["value"].IsString())
			{
				return false;
			}
			m_mapValues[std::chrono::milliseconds(key)] = std::string(temp["value"].GetString());
		}
		else if (m_type >= PropertyKeyframeTypeBase)
		{
			if (!temp["value"].IsNumber())
				return false;
			SPropertyKeyframeNode node;
			node.value = temp["value"].GetFloat();
			if (temp.HasMember("outCurve") && temp["outCurve"].IsInt())
				node.outCurve = static_cast<EPropertyKeyframeCurveMode>(temp["outCurve"].GetInt());
			if (temp.HasMember("leftHandle"))
				LoadHandleFromJson(temp["leftHandle"], node.leftHandle);
			if (temp.HasMember("rightHandle"))
				LoadHandleFromJson(temp["rightHandle"], node.rightHandle);
			m_mapKeyframeNodes[std::chrono::milliseconds(key)] = node;
		}
	}

	return true;
}

void CTimeLineObjectFx::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();
	root.AddMember(rapidjson::StringRef(TimeLineObjectFx_type_Json), m_type, allocator);
	rapidjson::Value jsonValues(rapidjson::kArrayType);
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	if (m_type >= PropertyKeyframeTypeBase)
	{
		for (const auto& it : m_mapKeyframeNodes)
		{
			rapidjson::Value temp(rapidjson::kObjectType);
			temp.AddMember("key", static_cast<int>(it.first.count()), allocator);
			temp.AddMember("value", it.second.value, allocator);
			temp.AddMember("outCurve", static_cast<int>(it.second.outCurve), allocator);
			rapidjson::Value leftHandle(rapidjson::kObjectType);
			SaveHandleToJson(it.second.leftHandle, leftHandle, allocator);
			temp.AddMember("leftHandle", leftHandle, allocator);
			rapidjson::Value rightHandle(rapidjson::kObjectType);
			SaveHandleToJson(it.second.rightHandle, rightHandle, allocator);
			temp.AddMember("rightHandle", rightHandle, allocator);
			jsonValues.PushBack(temp, allocator);
		}
	}
	else
	{
	for (const auto& it : m_mapValues)
	{
		rapidjson::Value temp(rapidjson::kObjectType);
		temp.AddMember("key", static_cast<int>(it.first.count()), allocator);
		if (m_type == ITimeLineObject::FxType_Scale)
		{
			if (std::holds_alternative<DirectX::XMFLOAT3>(it.second))
			{
				auto v1 = std::get<DirectX::XMFLOAT3>(it.second);
				rapidjson::Value v(rapidjson::kArrayType);
				v.PushBack(v1.x, allocator);
				v.PushBack(v1.y, allocator);
				v.PushBack(v1.z, allocator);
				temp.AddMember("value", v, allocator);
			}
		}
		else if (m_type == ITimeLineObject::FxType_Size
			|| m_type == ITimeLineObject::FxType_Fov
			|| m_type == ITimeLineObject::FxType_PosX
			|| m_type == ITimeLineObject::FxType_PosY
			|| m_type == ITimeLineObject::FxType_PosZ
			|| m_type == ITimeLineObject::FxType_RotX
			|| m_type == ITimeLineObject::FxType_RotY
			|| m_type == ITimeLineObject::FxType_RotZ)
		{
			if (std::holds_alternative<float>(it.second))
			{
				auto v1 = std::get<float>(it.second);
				temp.AddMember("value", v1, allocator);
			}
		}
		else if (m_type == ITimeLineObject::FxType_ObserveTarget)
		{
			if (std::holds_alternative<std::string>(it.second))
			{
				const auto& v1 = std::get<std::string>(it.second);
				temp.AddMember("value", rapidjson::Value(v1.c_str(), allocator), allocator);
			}
		}
		jsonValues.PushBack(temp, allocator);
	}
	}
	root.AddMember(rapidjson::StringRef(TimeLineObjectFx_values_Json), jsonValues, allocator);
}








CTimeLineObject::CTimeLineObject() 
{}

CTimeLineObject::CTimeLineObject(const QUuid& id, int type) :
	m_id(id),
	m_type(type)
{
}

CTimeLineObject::~CTimeLineObject()
{
}

CTimeLineObject::CTimeLineObject(const CTimeLineObject& other)
{
	m_id = other.m_id;
	m_type = other.m_type;
	m_duration = other.m_duration;
	m_startTime = other.m_startTime;
	m_displayName = other.m_displayName;
	m_bClipFxTrackVisible = other.m_bClipFxTrackVisible;
	m_bShowPath = other.m_bShowPath;

	for (auto& iter : other.m_hashFxs)
	{
		m_hashFxs[iter.first] = std::shared_ptr<ITimeLineObjectFx>(dynamic_cast<ITimeLineObjectFx*>(iter.second->Copy()));
	}
	for (auto& iter : other.m_hashPropertyKeyframes)
	{
		m_hashPropertyKeyframes[iter.first] = std::shared_ptr<CTimeLineObjectFx>(
			dynamic_cast<CTimeLineObjectFx*>(iter.second->Copy()));
	}
}

CTimeLineObject& CTimeLineObject::operator=(const CTimeLineObject& other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);

	m_id = other.m_id;
	m_type = other.m_type;
	m_duration = other.m_duration;
	m_startTime = other.m_startTime;
	m_displayName = other.m_displayName;
	m_bClipFxTrackVisible = other.m_bClipFxTrackVisible;
	m_bShowPath = other.m_bShowPath;

	m_hashFxs.clear();
	for (auto& iter : other.m_hashFxs)
	{
		m_hashFxs[iter.first] = std::shared_ptr<ITimeLineObjectFx>(dynamic_cast<ITimeLineObjectFx*>(iter.second->Copy()));
	}
	m_hashPropertyKeyframes.clear();
	for (auto& iter : other.m_hashPropertyKeyframes)
	{
		m_hashPropertyKeyframes[iter.first] = std::shared_ptr<CTimeLineObjectFx>(
			dynamic_cast<CTimeLineObjectFx*>(iter.second->Copy()));
	}

	return *this;
}

QString CTimeLineObject::GetDisplayName()
{
	return m_displayName;
}

void CTimeLineObject::SetDisplayName(const QString& name)
{
	m_displayName = name;
}

QUuid CTimeLineObject::GetId()
{
	return m_id;
}

int CTimeLineObject::GetType()
{
	return m_type;
}

std::chrono::milliseconds CTimeLineObject::GetStartTime()
{
	return m_startTime;
}

void CTimeLineObject::SetStartTime(const std::chrono::milliseconds& ms)
{
	// 起始时间帧对齐：Floor
	m_startTime = g_pConvertHelper->formatTimeWithFrame(ms, RoundingMode::Floor);
}

std::chrono::milliseconds CTimeLineObject::GetDuration()
{
	return m_duration;
}

void CTimeLineObject::SetDuration(const std::chrono::milliseconds& msPlayTime)
{
	// 时长帧对齐：Floor；正时长至少1帧
	auto aligned = g_pConvertHelper->formatTimeWithFrame(msPlayTime, RoundingMode::Floor);
	if (msPlayTime.count() > 0 && aligned.count() == 0)
		aligned = g_pConvertHelper->frameToTimes(1);
	m_duration = aligned;
}

void CTimeLineObject::AddFx(int type)
{
	std::shared_ptr<ITimeLineObjectFx> pFx = std::make_shared<CTimeLineObjectFx>(type);

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashFxs[type] = pFx;
}

void CTimeLineObject::DelereFx(int type)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashFxs.erase(type);
}

void CTimeLineObject::ClearFx()
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashFxs.clear();
}

std::weak_ptr<ITimeLineObjectFx> CTimeLineObject::GetFx(int type)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashFxs.find(type);
	if (iter != m_hashFxs.end())
		return iter->second;
	else
		return std::weak_ptr<ITimeLineObjectFx>();
}

std::weak_ptr<CTimeLineObjectFx> CTimeLineObject::GetPropertyKeyframeTrack(int propId) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashPropertyKeyframes.find(propId);
	if (iter != m_hashPropertyKeyframes.end())
		return iter->second;
	return std::weak_ptr<CTimeLineObjectFx>();
}

std::shared_ptr<CTimeLineObjectFx> CTimeLineObject::EnsurePropertyKeyframeTrack(int propId)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashPropertyKeyframes.find(propId);
	if (iter != m_hashPropertyKeyframes.end())
		return iter->second;

	auto pTrack = std::make_shared<CTimeLineObjectFx>(EncodePropertyKeyframeType(propId));
	m_hashPropertyKeyframes[propId] = pTrack;
	return pTrack;
}

void CTimeLineObject::AddPropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs, float value)
{
	auto pTrack = EnsurePropertyKeyframeTrack(propId);
	if (pTrack)
		pTrack->AddValue(clipLocalMs, value);
}

void CTimeLineObject::DeletePropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashPropertyKeyframes.find(propId);
	if (iter == m_hashPropertyKeyframes.end() || !iter->second)
		return;

	iter->second->DelereValue(clipLocalMs);
	if (iter->second->IsEmpty())
		m_hashPropertyKeyframes.erase(iter);

	// 关键帧全部清空后复位折叠，避免再次添加时因旧 Visible=true 自动展开
	bool bAny = false;
	for (const auto& it : m_hashPropertyKeyframes)
	{
		if (it.second && !it.second->IsEmpty())
		{
			bAny = true;
			break;
		}
	}
	if (!bAny)
		m_bClipFxTrackVisible = false;
}

bool CTimeLineObject::HasPropertyKeyframeAt(int propId, const std::chrono::milliseconds& clipLocalMs) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashPropertyKeyframes.find(propId);
	if (iter == m_hashPropertyKeyframes.end() || !iter->second)
		return false;
	return iter->second->HasValueAt(clipLocalMs);
}

std::vector<std::chrono::milliseconds> CTimeLineObject::GetPropertyKeyframeTimes(int propId) const
{
	std::vector<std::chrono::milliseconds> times;
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashPropertyKeyframes.find(propId);
	if (iter == m_hashPropertyKeyframes.end() || !iter->second)
		return times;

	auto values = iter->second->GetFloatKeyframes();
	times.reserve(values.size());
	for (const auto& it : values)
		times.push_back(it.first);
	return times;
}

std::vector<std::chrono::milliseconds> CTimeLineObject::GetAllPropertyKeyframeTimes() const
{
	std::set<std::chrono::milliseconds> uniqueTimes;
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (const auto& it : m_hashPropertyKeyframes)
	{
		if (!it.second || it.second->IsEmpty())
			continue;
		for (const auto& kf : it.second->GetFloatKeyframes())
			uniqueTimes.insert(kf.first);
	}
	return std::vector<std::chrono::milliseconds>(uniqueTimes.begin(), uniqueTimes.end());
}

bool CTimeLineObject::HasAnyPropertyKeyframes() const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (const auto& it : m_hashPropertyKeyframes)
	{
		if (it.second && !it.second->IsEmpty())
			return true;
	}
	return false;
}

bool CTimeLineObject::IsPropertyKeyframeTrackEmpty(int propId) const
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto iter = m_hashPropertyKeyframes.find(propId);
	if (iter == m_hashPropertyKeyframes.end() || !iter->second)
		return true;
	return iter->second->IsEmpty();
}

void CTimeLineObject::MovePropertyKeyframe(int propId, const std::chrono::milliseconds& oldMs, const std::chrono::milliseconds& newMs)
{
	auto pTrack = EnsurePropertyKeyframeTrack(propId);
	if (pTrack)
		pTrack->MoveKeyframe(oldMs, newMs);
}

void CTimeLineObject::SetPropertyKeyframeValue(int propId, const std::chrono::milliseconds& clipLocalMs, float value)
{
	auto pTrack = EnsurePropertyKeyframeTrack(propId);
	if (pTrack)
		pTrack->SetKeyframeValue(clipLocalMs, value);
}

void CTimeLineObject::SetPropertyKeyframeCurveMode(int propId, const std::chrono::milliseconds& clipLocalMs, EPropertyKeyframeCurveMode mode)
{
	if (IsPropertyKeyframeCurveModeLockedToLine(static_cast<EMediaPropertyId>(propId)))
		mode = EPropertyKeyframeCurveMode::Line;
	auto pTrack = EnsurePropertyKeyframeTrack(propId);
	if (pTrack)
		pTrack->SetKeyframeCurveMode(clipLocalMs, mode);
}

EPropertyKeyframeCurveMode CTimeLineObject::GetPropertyKeyframeCurveMode(int propId, const std::chrono::milliseconds& clipLocalMs) const
{
	if (IsPropertyKeyframeCurveModeLockedToLine(static_cast<EMediaPropertyId>(propId)))
		return EPropertyKeyframeCurveMode::Line;
	auto pTrack = GetPropertyKeyframeTrack(propId).lock();
	if (!pTrack)
		return EPropertyKeyframeCurveMode::Line;
	return pTrack->GetKeyframeCurveMode(clipLocalMs);
}

void CTimeLineObject::SetPropertyKeyframeHandle(int propId, const std::chrono::milliseconds& clipLocalMs, bool bLeft, const SPropertyKeyframeHandle& handle)
{
	if (IsPropertyKeyframeCurveModeLockedToLine(static_cast<EMediaPropertyId>(propId)))
		return;
	auto pTrack = EnsurePropertyKeyframeTrack(propId);
	if (pTrack)
		pTrack->SetKeyframeHandle(clipLocalMs, bLeft, handle);
}

void CTimeLineObject::CheckPropertyKeyframeHandles(int propId, const std::chrono::milliseconds& clipLocalMs)
{
	auto pTrack = EnsurePropertyKeyframeTrack(propId);
	if (pTrack)
		pTrack->CheckKeyframeHandles(clipLocalMs);
}

bool CTimeLineObject::GetPropertyKeyframeNode(int propId, const std::chrono::milliseconds& clipLocalMs, SPropertyKeyframeNode& outNode) const
{
	auto pTrack = GetPropertyKeyframeTrack(propId).lock();
	if (!pTrack)
		return false;
	return pTrack->GetKeyframeNodeAt(clipLocalMs, outNode);
}

std::chrono::milliseconds CTimeLineObject::GetPrePropertyKeyframeTime(int propId, const std::chrono::milliseconds& clipLocalMs) const
{
	auto pTrack = GetPropertyKeyframeTrack(propId).lock();
	if (!pTrack)
		return std::chrono::milliseconds(-1);
	return pTrack->GetPreKeyframeTime(clipLocalMs);
}

std::chrono::milliseconds CTimeLineObject::GetNextPropertyKeyframeTime(int propId, const std::chrono::milliseconds& clipLocalMs) const
{
	auto pTrack = GetPropertyKeyframeTrack(propId).lock();
	if (!pTrack)
		return std::chrono::milliseconds(-1);
	return pTrack->GetNextKeyframeTime(clipLocalMs);
}

bool CTimeLineObject::HasPrePropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs) const
{
	auto pTrack = GetPropertyKeyframeTrack(propId).lock();
	if (!pTrack)
		return false;
	return pTrack->HasPreKeyframe(clipLocalMs);
}

bool CTimeLineObject::HasNextPropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs) const
{
	auto pTrack = GetPropertyKeyframeTrack(propId).lock();
	if (!pTrack)
		return false;
	return pTrack->HasNextKeyframe(clipLocalMs);
}

#define TimeLineObject_StartTime_Json "TimeLineObject_StartTime"
#define TimeLineObject_Duration_Json "TimeLineObject_Duration"
#define TimeLineObject_id_Json "TimeLineObject_id"
#define TimeLineObject_type_Json "TimeLineObject_type"
#define TimeLineObject_DisplayName_Json "TimeLineObject_DisplayName"
#define TimeLineObject_Fxs_Json "TimeLineObject_Fxs"
#define TimeLineObject_PropertyKeyframes_Json "TimeLineObject_PropertyKeyframes"
#define TimeLineObject_ClipFxTrackVisible_Json "TimeLineObject_ClipFxTrackVisible"
#define TimeLineObject_ShowPath_Json "TimeLineObject_ShowPath"

bool CTimeLineObject::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
	{
		return false;
	}
	if (!root.HasMember(TimeLineObject_StartTime_Json) || !root[TimeLineObject_StartTime_Json].IsInt())
	{
		return false;
	}
	m_startTime = g_pConvertHelper->formatTimeWithFrame(
		std::chrono::milliseconds(root[TimeLineObject_StartTime_Json].GetInt()), RoundingMode::Floor);

	if (!root.HasMember(TimeLineObject_Duration_Json) || !root[TimeLineObject_Duration_Json].IsInt())
	{
		return false;
	}
	{
		auto dur = std::chrono::milliseconds(root[TimeLineObject_Duration_Json].GetInt());
		auto aligned = g_pConvertHelper->formatTimeWithFrame(dur, RoundingMode::Floor);
		if (dur.count() > 0 && aligned.count() == 0)
			aligned = g_pConvertHelper->frameToTimes(1);
		m_duration = aligned;
	}

	if (!root.HasMember(TimeLineObject_id_Json) || !root[TimeLineObject_id_Json].IsString())
	{
		return false;
	}
	m_id = QUuid::fromString(QString::fromUtf8(root[TimeLineObject_id_Json].GetString()));

	if (!root.HasMember(TimeLineObject_type_Json) || !root[TimeLineObject_type_Json].IsInt())
	{
		return false;
	}
	m_type = root[TimeLineObject_type_Json].GetInt();

	m_displayName.clear();
	if (root.HasMember(TimeLineObject_DisplayName_Json) && root[TimeLineObject_DisplayName_Json].IsString())
		m_displayName = QString::fromUtf8(root[TimeLineObject_DisplayName_Json].GetString());

	if (!root.HasMember(TimeLineObject_Fxs_Json) || !root[TimeLineObject_Fxs_Json].IsArray())
	{
		return false;
	}
	const auto& jsonFxs = root[TimeLineObject_Fxs_Json];

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashFxs.clear();
	for (const auto& jsonFx : jsonFxs.GetArray())
	{
		auto p = std::shared_ptr<ITimeLineObjectFx>(new CTimeLineObjectFx());
		if (p->LoadDataFromRapidJson(jsonFx))
		{
			m_hashFxs.insert({ p->GetType(),p });
		}
		else
		{
			return false;
		}
	}

	// 打开工程时始终折叠 ClipFx，需用户手动「显示关键帧」；不恢复上次打开态
	m_bClipFxTrackVisible = false;

	m_bShowPath = true;
	if (root.HasMember(TimeLineObject_ShowPath_Json) && root[TimeLineObject_ShowPath_Json].IsBool())
		m_bShowPath = root[TimeLineObject_ShowPath_Json].GetBool();

	m_hashPropertyKeyframes.clear();
	if (root.HasMember(TimeLineObject_PropertyKeyframes_Json) && root[TimeLineObject_PropertyKeyframes_Json].IsArray())
	{
		for (const auto& jsonPropFx : root[TimeLineObject_PropertyKeyframes_Json].GetArray())
		{
			if (!jsonPropFx.IsObject())
				continue;
			if (!jsonPropFx.HasMember("propId") || !jsonPropFx["propId"].IsInt())
				continue;
			int propId = jsonPropFx["propId"].GetInt();
			auto pTrack = std::make_shared<CTimeLineObjectFx>();
			if (pTrack->LoadDataFromRapidJson(jsonPropFx))
				m_hashPropertyKeyframes[propId] = pTrack;
		}
	}

	return true;
}

bool CTimeLineObject::ReplacePropertyKeyframesFromRapidJson(const rapidjson::Value& propertyKeyframesArray)
{
	if (!propertyKeyframesArray.IsArray())
		return false;

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashPropertyKeyframes.clear();
	for (const auto& jsonPropFx : propertyKeyframesArray.GetArray())
	{
		if (!jsonPropFx.IsObject())
			continue;
		if (!jsonPropFx.HasMember("propId") || !jsonPropFx["propId"].IsInt())
			continue;
		int propId = jsonPropFx["propId"].GetInt();
		auto pTrack = std::make_shared<CTimeLineObjectFx>();
		if (pTrack->LoadDataFromRapidJson(jsonPropFx))
			m_hashPropertyKeyframes[propId] = pTrack;
	}
	return true;
}

void CTimeLineObject::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();
	root.AddMember(rapidjson::StringRef(TimeLineObject_StartTime_Json), static_cast<int>(m_startTime.count()), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_Duration_Json), static_cast<int>(m_duration.count()), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_id_Json), rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_type_Json), m_type, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_DisplayName_Json), rapidjson::Value(m_displayName.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_ClipFxTrackVisible_Json), m_bClipFxTrackVisible, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_ShowPath_Json), m_bShowPath, allocator);

	rapidjson::Value jsonFxs(rapidjson::kArrayType);
	rapidjson::Value jsonPropFxs(rapidjson::kArrayType);
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (const auto& it : m_hashFxs)
	{
		rapidjson::Value fx(rapidjson::kObjectType);
		it.second->SaveDataToRapidJson(fx, allocator);
		jsonFxs.PushBack(fx, allocator);
	}
	for (const auto& it : m_hashPropertyKeyframes)
	{
		if (!it.second || it.second->IsEmpty())
			continue;
		rapidjson::Value propFx(rapidjson::kObjectType);
		propFx.AddMember("propId", it.first, allocator);
		rapidjson::Value trackData(rapidjson::kObjectType);
		it.second->SaveDataToRapidJson(trackData, allocator);
		for (auto mIt = trackData.MemberBegin(); mIt != trackData.MemberEnd(); ++mIt)
		{
			propFx.AddMember(mIt->name, mIt->value, allocator);
		}
		jsonPropFxs.PushBack(propFx, allocator);
	}
	root.AddMember(rapidjson::StringRef(TimeLineObject_Fxs_Json), jsonFxs, allocator);
	root.AddMember(rapidjson::StringRef(TimeLineObject_PropertyKeyframes_Json), jsonPropFxs, allocator);
}
