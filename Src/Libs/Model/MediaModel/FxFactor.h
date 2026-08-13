#pragma once
#include "MediaModelGlobal.h"
#include "Common/IFxInterface.h"
#include "Common/XEffectCommon.h"


class MEDIAMODEL_EXPORT CFxFactor : public IFxFactor, public std::enable_shared_from_this<CFxFactor>
{
public:
	CFxFactor(int type);
	virtual ~CFxFactor();

	CFxFactor(const CFxFactor& data);					 // 拷贝构造
	CFxFactor& operator=(const CFxFactor& data);		 // 拷贝赋值

	CFxFactor& operator=(std::shared_ptr<IFxFactor> pdata);		 // 拷贝赋值
	CFxFactor(std::shared_ptr<IFxFactor> pdata);		 // 构造

public:
	virtual int GetType();

	virtual std::shared_ptr<SFxFactor> GetData();
	virtual void SetData(std::shared_ptr<SFxFactor> factor);

	virtual QString GetName();

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual void LoadDataFromRapidJson(const rapidjson::Value &root) override;
protected:
	int m_type;
	QString m_name;
	std::shared_ptr<SFxFactor> m_pdata = nullptr;
};