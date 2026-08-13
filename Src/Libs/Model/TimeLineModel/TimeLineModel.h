#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <list>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include "TimeLineSection.h"
#include "TimeLineTag.h"
#include "TimeLineNote.h"
#include "TimeLineTransFx.h"

class CTimeLineModel : public ITimeLineModel
{
	INIT_ICOPYINTERFACE(CTimeLineModel)
public:
	CTimeLineModel();
	virtual ~CTimeLineModel();
	CTimeLineModel(const CTimeLineModel &other);
	CTimeLineModel& operator=(const CTimeLineModel &other);

	virtual QUuid GetId()override;
	virtual void SetId(const QUuid& id)override;

	virtual void ResetModel()override;

	virtual std::shared_ptr<ITimeLineObject> CreateObject(const QUuid& id, int type)override;
	virtual std::vector<std::weak_ptr<ITimeLineObject>> GetObjects(const std::chrono::milliseconds& ms, bool bReverse = false)override;
	virtual std::vector<std::weak_ptr<ITimeLineObject>> GetPreObjects(const std::chrono::milliseconds& ms)override;
	virtual std::weak_ptr<ITimeLineObject> GetTimeLineObject(const QUuid& id)override;
	virtual std::vector<QUuid> GetObjectIds(const std::chrono::milliseconds& ms, int type = -1, int Order = OrderType_Random)override;
	virtual std::vector<QUuid> GetAllObjectIds(int type = -1)override;
	virtual bool HasTimelineObjects()override;

	virtual std::shared_ptr<ITimeLineLayer> CreateLayer()override;
	virtual int GetNewLayerIndex()override;
	virtual void AddLayer(std::shared_ptr<ITimeLineLayer> p,int index)override;
	virtual void MoveLayer(const QUuid& id, int index)override;
	virtual void DeleteLayer(const QUuid& id)override;
	virtual int GetLayerCount()override;
	virtual std::vector<QUuid> GetAllLayerIDs()override;
	virtual std::weak_ptr<ITimeLineLayer> GetLayer(const QUuid& id)override;
	virtual int GetLayerIndex(const QUuid& id)override;

	virtual std::chrono::milliseconds GetDuration()override;
	virtual void SetDuration(const std::chrono::milliseconds& msPlayTime)override;

	virtual int GetPlayEndActionType()override;
	virtual void SetPlayEndActionType(int type)override;

	virtual int GetSectionPlayMode()override;
	virtual void SetSectionPlayMode(int mode)override;

	virtual std::chrono::milliseconds GetMaxUsefulDuration()override;
	virtual void SetMaxUsefulDuration()override;

	virtual const std::vector<std::shared_ptr<ITimeLineSection>> GetAllSections()override;
	virtual std::weak_ptr<ITimeLineSection> GetSection(QUuid& id)override;
	virtual std::weak_ptr<ITimeLineSection> GetSection(const std::chrono::milliseconds& ms) override;
	virtual QUuid GetPreSectionId(QUuid id) override;
	virtual QUuid GetNextSectionId(QUuid id) override;
	virtual QUuid GetFirstSectionId() override;
	virtual void InitSection()override;
	virtual std::pair<QUuid, QUuid> SplitSection(const std::chrono::milliseconds& splitTime) override;
	virtual std::pair<QUuid, QUuid> MergeSection(const std::chrono::milliseconds& mergeTime) override;
	virtual void DelSection(std::shared_ptr<ITimeLineSection> ppItem) override;
	virtual void DelSection(QUuid id) override;
	virtual void DoModifySection(std::chrono::milliseconds& offset, std::shared_ptr<ITimeLineSection>operateSection, int type) override;
	virtual bool ReplaceAllSectionsFromRapidJson(const rapidjson::Value& sectionsArray) override;
	virtual bool InsertTime(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length, ETimeEditScope scope) override;
	virtual bool RemoveTime(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length, ETimeEditScope scope) override;

	virtual std::shared_ptr<ITimeLineTag> CreateTag() override;
	virtual void AddTag(std::shared_ptr<ITimeLineTag> pTag) override;
	virtual void DeleteTag(const QUuid& id) override;
	virtual std::weak_ptr<ITimeLineTag> GetTag(const QUuid& id) override;
	virtual const std::vector<std::shared_ptr<ITimeLineTag>>& GetAllTags() override;

	virtual std::shared_ptr<ITimeLineNote> CreateNote() override;
	virtual void AddNote(std::shared_ptr<ITimeLineNote> pNote) override;
	virtual void DeleteNote(const QUuid& id) override;
	virtual std::weak_ptr<ITimeLineNote> GetNote(const QUuid& id) override;
	virtual const std::vector<std::shared_ptr<ITimeLineNote>>& GetAllNotes() override;

	virtual std::shared_ptr<ITimeLineTransFx> CreateTransFx() override;
	virtual void AddTransFx(std::shared_ptr<ITimeLineTransFx> pTransFx) override;
	virtual void DeleteTransFx(const QUuid& id) override;
	virtual void DeleteTransFxByObject(const QUuid& objectId) override;
	virtual std::weak_ptr<ITimeLineTransFx> GetTransFx(const QUuid& id) override;
	virtual std::weak_ptr<ITimeLineTransFx> FindTransFxByObjects(const QUuid& leftId, const QUuid& rightId) override;
	virtual const std::vector<std::shared_ptr<ITimeLineTransFx>>& GetAllTransFxs() override;
	virtual std::vector<std::shared_ptr<ITimeLineTransFx>> GetTransFxsByObject(const QUuid& objectId) override;
	virtual void RebuildTransFxFromObjects() override;

	virtual float GetTimeLineFps() override;
	virtual void SetTimeLineFps(float fps) override;
	virtual bool GetTimeLineDropFrame() override;
	virtual void SetTimeLineDropFrame(bool dropFrame) override;

	virtual void SetNoniusPos(const std::chrono::milliseconds& ms) override;
	virtual std::chrono::milliseconds GetNoniusPos() override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root)override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)override;

protected:
	QString GetFirstNoRepeatSectionName();
	QString GetFirstNoRepeatTagName();
	QString GetFirstNoRepeatNoteName();

private:
	// 将毫秒时间对齐到帧边界（Floor）
	std::chrono::milliseconds normalizeTimeMs(std::chrono::milliseconds ms) const;
	// 将时长对齐到帧边界（Floor）；输入>0时结果至少1帧，避免亚帧时长塌缩为0
	std::chrono::milliseconds normalizeDurationMs(std::chrono::milliseconds ms) const;
	// 判断指定时间点是否有素材
	bool hasClipAtPosition(const std::chrono::milliseconds& position) const;
	// 计算所有图层素材最大结束时间
	std::chrono::milliseconds calcMaxUsefulDurationUnlocked() const;
	// 图层插入时间
	void rippleInsertOnLayers(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length);
	// 图层移除时间
	void rippleRemoveOnLayers(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length);
	// Section插入时间
	void adjustSectionsForInsert(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length);
	// Section移除时间
	void adjustSectionsForRemove(const std::chrono::milliseconds& position, const std::chrono::milliseconds& length);
	// 同步最后 Section 结束时间到时间线总长
	void syncLastSectionEndToDurationUnlocked();

private:
	int m_nSectionCount = 1;
	int m_nTagCount = 1;
	int m_nNoteCount = 1;

private:
	
	std::shared_mutex	m_mtx;
	QUuid m_id;
	std::unordered_map<QUuid, std::shared_ptr<ITimeLineLayer>, SUuidHashFunc>	m_hashLayers;
	std::vector<QUuid> m_vecLayers;
	std::list<std::shared_ptr<ITimeLineSection>> m_listSections;
	std::vector<std::shared_ptr<ITimeLineTag>> m_vecTags;
	std::vector<std::shared_ptr<ITimeLineNote>> m_vecNotes;
	std::vector<std::shared_ptr<ITimeLineTransFx>> m_vecTransFxs;

	std::chrono::milliseconds m_duration{7200000};
	int m_playEndActionType = 0;
	int m_sectionPlayMode = ITimeLineModel::SectionPlayMode_Play;

	int m_nLayerIndex = 0;

	float m_fTimeLineFps = 30.0f;           // 时间线帧率
	bool m_bTimeLineDropFrame = false;      // 是否丢帧（29.97 DF）

	std::chrono::milliseconds m_noniusPos = 0ms; // 游标位置
};