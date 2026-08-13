#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <shared_mutex>

class CTimeLineNote : public ITimeLineNote
{
	INIT_ICOPYINTERFACE(CTimeLineNote)
public:
	CTimeLineNote();
	CTimeLineNote(const QUuid& id);
	virtual ~CTimeLineNote();
	CTimeLineNote(const CTimeLineNote& other);
	CTimeLineNote& operator=(const CTimeLineNote& other);
	bool operator<(const CTimeLineNote& other);

	virtual QUuid GetId() override;
	virtual void SetId(const QUuid& id) override;

	virtual QString GetNoteName() override;
	virtual void SetNoteName(const QString& name) override;

	virtual std::chrono::milliseconds GetNotePostion() override;
	virtual void SetNotePostion(const std::chrono::milliseconds& time) override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) override;

private:
	std::shared_mutex m_mtx;
	QUuid m_id;
	QString m_noteName;
	// 时间轴上的位置
	std::chrono::milliseconds m_location{ 0 };
};
