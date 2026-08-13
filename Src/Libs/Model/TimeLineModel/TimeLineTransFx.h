#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <shared_mutex>

class CTimeLineTransFx : public ITimeLineTransFx
{
	INIT_ICOPYINTERFACE(CTimeLineTransFx)
public:
	CTimeLineTransFx();
	CTimeLineTransFx(const QUuid& id);
	virtual ~CTimeLineTransFx();
	CTimeLineTransFx(const CTimeLineTransFx& other);
	CTimeLineTransFx& operator=(const CTimeLineTransFx& other);

	virtual QUuid GetId() override;
	virtual void SetId(const QUuid& id) override;

	virtual QUuid GetLayerId() override;
	virtual void SetLayerId(const QUuid& id) override;

	virtual QUuid GetLeftObjectId() override;
	virtual void SetLeftObjectId(const QUuid& id) override;

	virtual QUuid GetRightObjectId() override;
	virtual void SetRightObjectId(const QUuid& id) override;

	virtual std::chrono::milliseconds GetDuration() override;
	virtual void SetDuration(const std::chrono::milliseconds& dur) override;

	virtual int GetTransType() override;
	virtual void SetTransType(int type) override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) override;

private:
	std::shared_mutex m_mtx;
	QUuid m_id;
	QUuid m_layerId;
	QUuid m_leftObjectId;
	QUuid m_rightObjectId;
	std::chrono::milliseconds m_duration{ 0 };
	int m_transType = 0;
};
