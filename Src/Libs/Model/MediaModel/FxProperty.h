#pragma once
#include "MediaModelGlobal.h"
#include "Common/IMediaPropertyInterface.h"

class MEDIAMODEL_EXPORT CFxProperty : public IFxProperty, public std::enable_shared_from_this<CFxProperty>
{
public:
	CFxProperty();
	virtual ~CFxProperty();

	CFxProperty(const CFxProperty& data);					 // 拷贝构造
	CFxProperty& operator=(const CFxProperty& data);		 // 拷贝赋值

	CFxProperty(std::shared_ptr<IFxProperty> pdata);		 // 构造

public:
	virtual void ClearAllFxFacyor();
	virtual void DeleteFxFactor(int type);
	virtual void AddFxFaxtor(std::shared_ptr<IFxFactor> pFxFactor, int nIndex = -1);
	virtual std::shared_ptr<IFxFactor> GetFxFactor(int type);
	virtual std::shared_ptr<IFxFactor> GetFxFactorWithIndex(int type, int& nIndex);

	virtual std::vector<std::shared_ptr<IFxFactor>> GetAllFxFactor();

	virtual bool hasFactor();

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual void LoadDataFromRapidJson(const rapidjson::Value &root) override;

	virtual void CopyProperty(std::shared_ptr<IFxProperty> pNode) override;

private:
	std::vector<std::shared_ptr<IFxFactor>> m_vecFactor;
};