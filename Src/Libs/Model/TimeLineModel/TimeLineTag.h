#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <shared_mutex>

class CTimeLineTag : public ITimeLineTag
{
	INIT_ICOPYINTERFACE(CTimeLineTag)
public:
	CTimeLineTag();
	CTimeLineTag(const QUuid& id);
	virtual ~CTimeLineTag();
	CTimeLineTag(const CTimeLineTag& other);
	CTimeLineTag& operator=(const CTimeLineTag& other);

	virtual QUuid GetId() override;
	virtual void SetId(const QUuid& id) override;

	virtual QString GetTagName() override;
	virtual void SetTagName(const QString& name) override;

	virtual std::chrono::milliseconds GetTagPosition() override;
	virtual void SetTagPosition(const std::chrono::milliseconds& time) override;

	virtual std::chrono::milliseconds GetTagTCPosition() override;
	virtual void SetTagTCPosition(const std::chrono::milliseconds& time) override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) override;

private:
	std::shared_mutex m_mtx;
	QUuid m_id;
	QString m_tagName;
	std::chrono::milliseconds m_tagPosition{ 0 };
	std::chrono::milliseconds m_tagTCPosition{ 0 };
};