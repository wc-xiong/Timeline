#pragma once
#include "ICopyInterface.h"
#include "Common/Common/Maths/Point.h"
#include <list>
#include<QUuid>
#include<memory>
#include<chrono>
#include<vector>
#include<QColor>
#include <variant>
#include <string>
#include <DirectXMath.h>
#include <rapidjson/document.h>

// Windows.h 将 GetObject 宏化为 GetObjectW，破坏 ITimeLineLayer::GetObject
#ifdef GetObject
#undef GetObject
#endif

using FxData = std::variant<int, float, std::string, DirectX::XMFLOAT3>;

enum class ETimeEditScope
{
	LayersOnly,
	LayersAndSection,
};

class ITimeLineObjectFx :public ICopyInterface
{
public:
	virtual ~ITimeLineObjectFx() = default;

	virtual int GetType() = 0;

	virtual void AddValue(const std::chrono::milliseconds& ms, const FxData& v) = 0;
	virtual void DelereValue(const std::chrono::milliseconds& ms) = 0;
	virtual void ClearValue() = 0;

	virtual FxData GetValue(const std::chrono::milliseconds& ms) = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineObject :public ICopyInterface
{
public:
	virtual ~ITimeLineObject() = default;

	virtual QUuid GetId() = 0;

	enum EObjectType
	{
		ObjectType_None = -1,
		ObjectType_Media,
		ObjectType_3DModel,
	};
	virtual int GetType() = 0;

	virtual std::chrono::milliseconds GetStartTime() = 0;
	virtual void SetStartTime(const std::chrono::milliseconds& ms) = 0;

	virtual std::chrono::milliseconds GetDuration() = 0;
	virtual void SetDuration(const std::chrono::milliseconds& msPlayTime) = 0;

	enum EFxType
	{
		FxType_PosX,			// 位置 X（float）
		FxType_PosY,			// 位置 Y（float）
		FxType_PosZ,			// 位置 Z（float）
		FxType_RotX,			// 旋转 X（float，弧度）
		FxType_RotY,			// 旋转 Y（float，弧度）
		FxType_RotZ,			// 旋转 Z（float，弧度）
		FxType_Scale,			// 缩放（XMFLOAT3）
		FxType_Size,
		FxType_Fov,				// 相机垂直视场角（float）
		FxType_ObserveTarget,	// 观察目标物体 ID（string，空串表示无绑定）
	};
	virtual void AddFx(int type) = 0;
	virtual void DelereFx(int type) = 0;
	virtual void ClearFx() = 0;
	virtual std::weak_ptr<ITimeLineObjectFx> GetFx(int type) = 0;

	// 片段显示名（路径相机片段等）
	virtual QString GetDisplayName() = 0;
	virtual void SetDisplayName(const QString& name) = 0;

	/** @brief 路径相机片段是否在 3D 视图显示路径折线 */
	virtual bool IsShowPath() const = 0;
	/** @brief 设置路径相机片段是否显示路径折线 */
	virtual void SetShowPath(bool b) = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineLayer :public ICopyInterface
{
public:
	virtual ~ITimeLineLayer() = default;

	virtual QUuid GetId() = 0;

	virtual std::weak_ptr<ITimeLineObject> GetObject(const std::chrono::milliseconds& ms, bool bCheck = true) = 0;

	virtual void AddObject(const QUuid& tlId, std::shared_ptr<ITimeLineObject> p) = 0;
	virtual void MoveObject(const QUuid& tlId, const QUuid& id) = 0;
	virtual void DeleteObject(const QUuid& tlId, const QUuid& id) = 0;
	// 仅变更图层内部容器，不触发 Add/Delete 业务信号，供跨图层迁移复用
	virtual bool InsertObject(std::shared_ptr<ITimeLineObject> p) = 0;
	virtual bool ExtractObject(const QUuid& id, std::shared_ptr<ITimeLineObject>& outObject) = 0;
	virtual void ClearObjects() = 0;
	virtual int GetObjectCount() = 0;
	virtual bool HasTimelineObjects() = 0;
	virtual bool IsObjectExist(const QUuid& id) = 0;
	virtual std::vector<QUuid> GetAllObjectIDs() = 0;
	virtual std::weak_ptr<ITimeLineObject> GetObject(const QUuid& id) = 0;

	virtual std::chrono::milliseconds GetMaxTime() = 0;

	virtual QString GetName() = 0;
	virtual void SetName(const QString& name) = 0;

	virtual bool IsLock() = 0;
	virtual void SetLock(bool b) = 0;

	virtual bool IsHide() = 0;
	virtual void SetHide(bool b) = 0;

	virtual bool IsMute() = 0;
	virtual void SetMute(bool b) = 0;

	virtual QColor GetColor() = 0;
	virtual void SetColor(const QColor& v) = 0;

	virtual bool IsShowPlayProgress() = 0;
	virtual void SetShowPlayProgress(bool b) = 0;
	virtual QUuid GetAudioMachineId() = 0;
	virtual QString GetAudioMachineName() = 0;
	virtual void SetAudioMachine(const QUuid& machineId, const QString& machineName) = 0;
	virtual QString GetAudioDeviceKey() = 0;
	virtual QString GetAudioDeviceName() = 0;
	virtual void SetAudioDevice(const QString& deviceKey, const QString& deviceName) = 0;
	virtual void ClearAudioDevice() = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineSection : public ICopyInterface
{
public:
	virtual ~ITimeLineSection() = default;
	virtual QString GetSectionName() = 0;
	virtual QUuid GetId() = 0;
	virtual void SetSectionName(const QString& name) = 0;
	virtual void SetSectionStartTime(const std::chrono::milliseconds& time) = 0;
	virtual std::chrono::milliseconds GetSectionStartTime() = 0;
	virtual std::chrono::milliseconds GetSectionEndTime() = 0;

	// 规则：左闭右开 [,)
	virtual std::chrono::milliseconds GetSectionValidEndTime() = 0;
	virtual void SetSectionEndTime(const std::chrono::milliseconds& time) = 0;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineTag : public ICopyInterface
{
public:
	virtual ~ITimeLineTag() = default;

	virtual QUuid GetId() = 0;
	virtual void SetId(const QUuid& id) = 0;

	virtual QString GetTagName() = 0;
	virtual void SetTagName(const QString& name) = 0;

	virtual std::chrono::milliseconds GetTagPosition() = 0;
	virtual void SetTagPosition(const std::chrono::milliseconds& time) = 0;

	virtual std::chrono::milliseconds GetTagTCPosition() = 0;
	virtual void SetTagTCPosition(const std::chrono::milliseconds& time) = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineNote : public ICopyInterface
{
public:
	virtual ~ITimeLineNote() = default;

	virtual QUuid GetId() = 0;
	virtual void SetId(const QUuid& id) = 0;

	virtual QString GetNoteName() = 0;
	virtual void SetNoteName(const QString& name) = 0;

	virtual std::chrono::milliseconds GetNotePostion() = 0;
	virtual void SetNotePostion(const std::chrono::milliseconds& time) = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineTransFx : public ICopyInterface
{
public:
	virtual ~ITimeLineTransFx() = default;

	virtual QUuid GetId() = 0;
	virtual void SetId(const QUuid& id) = 0;

	virtual QUuid GetLayerId() = 0;
	virtual void SetLayerId(const QUuid& id) = 0;

	virtual QUuid GetLeftObjectId() = 0;
	virtual void SetLeftObjectId(const QUuid& id) = 0;

	virtual QUuid GetRightObjectId() = 0;
	virtual void SetRightObjectId(const QUuid& id) = 0;

	virtual std::chrono::milliseconds GetDuration() = 0;
	virtual void SetDuration(const std::chrono::milliseconds& dur) = 0;

	virtual int GetTransType() = 0;
	virtual void SetTransType(int type) = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

class ITimeLineModel :public ICopyInterface
{
public:
	virtual ~ITimeLineModel() = default;

	virtual QUuid GetId() = 0;
	virtual void SetId(const QUuid& id) = 0;

	virtual void ResetModel() = 0;

	virtual std::shared_ptr<ITimeLineObject> CreateObject(const QUuid& id, int type) = 0;
	virtual std::vector<std::weak_ptr<ITimeLineObject>> GetObjects(const std::chrono::milliseconds& ms, bool bReverse = false) = 0;
	virtual std::vector<std::weak_ptr<ITimeLineObject>> GetPreObjects(const std::chrono::milliseconds& ms) = 0;
	virtual std::weak_ptr<ITimeLineObject> GetTimeLineObject(const QUuid& id) = 0;

	enum EOrderType
	{
		OrderType_Descending = -1,
		OrderType_Random,
		OrderType_Ascending,
	};
	virtual std::vector<QUuid> GetObjectIds(const std::chrono::milliseconds& ms, int type = -1, int Order = OrderType_Random) = 0;

	virtual std::vector<QUuid> GetAllObjectIds(int type = -1) = 0;
	virtual bool HasTimelineObjects() = 0;

	virtual std::shared_ptr<ITimeLineLayer> CreateLayer() = 0;
	virtual int GetNewLayerIndex() = 0;
	virtual void AddLayer(std::shared_ptr<ITimeLineLayer> p, int index = -1) = 0;
	virtual void MoveLayer(const QUuid& id, int index) = 0;
	virtual void DeleteLayer(const QUuid& id) = 0;
	virtual int GetLayerCount() = 0;
	virtual std::vector<QUuid> GetAllLayerIDs() = 0;
	virtual std::weak_ptr<ITimeLineLayer> GetLayer(const QUuid& id) = 0;
	virtual int GetLayerIndex(const QUuid& id) = 0;

	virtual std::chrono::milliseconds GetDuration() = 0;
	virtual void SetDuration(const std::chrono::milliseconds& msPlayTime) = 0;

	enum EPlayEndActionType
	{
		PlayEndActionType_Cycle,
		PlayEndActionType_Stop,
		PlayEndActionType_Pause,
	};

	virtual int GetPlayEndActionType() = 0;
	virtual void SetPlayEndActionType(int type) = 0;

	enum ESectionPlayModeType
	{
		SectionPlayMode_Play = 0,
		SectionPlayMode_EndOfSection = 1,
		SectionPlayMode_LoopSection = 2,
	};

	virtual int GetSectionPlayMode() = 0;
	virtual void SetSectionPlayMode(int mode) = 0;

	virtual std::chrono::milliseconds GetMaxUsefulDuration() = 0;
	virtual void SetMaxUsefulDuration() = 0;

	virtual const std::vector<std::shared_ptr<ITimeLineSection>> GetAllSections() = 0;
	virtual std::weak_ptr<ITimeLineSection> GetSection(QUuid& id) = 0;
	virtual std::weak_ptr<ITimeLineSection> GetSection(const std::chrono::milliseconds& ms) = 0;
	virtual QUuid GetPreSectionId(QUuid id) = 0;
	virtual QUuid GetNextSectionId(QUuid id) = 0;
	virtual QUuid GetFirstSectionId() = 0;
	virtual void InitSection() = 0;
	virtual std::pair<QUuid, QUuid> SplitSection(const std::chrono::milliseconds& splitTime) = 0;
	virtual std::pair<QUuid, QUuid> MergeSection(const std::chrono::milliseconds& mergeTime) = 0;
	virtual void DelSection(std::shared_ptr<ITimeLineSection> ppItem) = 0;
	virtual void DelSection(QUuid id) = 0;
	virtual void DoModifySection(std::chrono::milliseconds& offset, std::shared_ptr<ITimeLineSection>operateSection, int type) = 0;

	/*
	* @brief 反序列化解析时间线Sections列表数据
	* 目前仅用于播放端使用
	*/
	virtual bool ReplaceAllSectionsFromRapidJson(const rapidjson::Value& sectionsArray) = 0;
	virtual bool InsertTime(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length, ETimeEditScope scope) = 0;
	virtual bool RemoveTime(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length, ETimeEditScope scope) = 0;

	virtual std::shared_ptr<ITimeLineTag> CreateTag() = 0;
	virtual void AddTag(std::shared_ptr<ITimeLineTag> pTag) = 0;
	virtual void DeleteTag(const QUuid& id) = 0;
	virtual std::weak_ptr<ITimeLineTag> GetTag(const QUuid& id) = 0;
	virtual const std::vector<std::shared_ptr<ITimeLineTag>>& GetAllTags() = 0;

	virtual std::shared_ptr<ITimeLineNote> CreateNote() = 0;
	virtual void AddNote(std::shared_ptr<ITimeLineNote> pNote) = 0;
	virtual void DeleteNote(const QUuid& id) = 0;
	virtual std::weak_ptr<ITimeLineNote> GetNote(const QUuid& id) = 0;
	virtual const std::vector<std::shared_ptr<ITimeLineNote>>& GetAllNotes() = 0;

	virtual std::shared_ptr<ITimeLineTransFx> CreateTransFx() = 0;
	virtual void AddTransFx(std::shared_ptr<ITimeLineTransFx> pTransFx) = 0;
	virtual void DeleteTransFx(const QUuid& id) = 0;
	virtual void DeleteTransFxByObject(const QUuid& objectId) = 0;
	virtual std::weak_ptr<ITimeLineTransFx> GetTransFx(const QUuid& id) = 0;
	virtual std::weak_ptr<ITimeLineTransFx> FindTransFxByObjects(const QUuid& leftId, const QUuid& rightId) = 0;
	virtual const std::vector<std::shared_ptr<ITimeLineTransFx>>& GetAllTransFxs() = 0;
	virtual std::vector<std::shared_ptr<ITimeLineTransFx>> GetTransFxsByObject(const QUuid& objectId) = 0;
	virtual void RebuildTransFxFromObjects() = 0;

	virtual float GetTimeLineFps() = 0;
	virtual void SetTimeLineFps(float fps) = 0;
	virtual bool GetTimeLineDropFrame() = 0;
	virtual void SetTimeLineDropFrame(bool dropFrame) = 0;

	virtual void SetNoniusPos(const std::chrono::milliseconds& ms) = 0;
	virtual std::chrono::milliseconds GetNoniusPos() = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
};

#ifdef GetObject
#undef GetObject
#endif
