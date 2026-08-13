#pragma once
#include <rapidjson/document.h>
#include <QString>
#include <memory>
struct SFxFactor;
//////////////////////////////////////////////////////////////////////////
/**
 * @brief XEffect 特效因子接口
 *
 * 每个特效因子（如裁剪、羽化、抠像、蒙版等）实现此接口，
 * 提供类型标识、数据读写、序列化及显示名称等统一访问方式。
 * 由 IFxProperty 以列表形式管理多个 IFxFactor 实例。
 */
class IFxFactor
{
public:
	virtual ~IFxFactor() {};

	virtual int GetType() = 0;

	virtual std::shared_ptr<SFxFactor> GetData() = 0;
	virtual void SetData(std::shared_ptr<SFxFactor> factor) = 0;
	virtual QString GetName() = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
	virtual void LoadDataFromRapidJson(const rapidjson::Value& root) = 0;
};