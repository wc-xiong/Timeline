#pragma once
#include "MediaModelGlobal.h"
#include "Common/Common/IMediaInterface.h"

#include <map>
#include <shared_mutex>


class MEDIAMODEL_EXPORT CMediaModel : public IMediaModel
{
	INIT_ICOPYINTERFACE(CMediaModel)
public:
	CMediaModel();
	virtual ~CMediaModel();
	CMediaModel(const CMediaModel &other);
	CMediaModel& operator=(const CMediaModel &other);

	//获取指定媒体,获取所有媒体的QUuid
	virtual std::vector<QUuid> GetAllMediaId();
	virtual std::weak_ptr<IMediaNode> GetMediaNode(const QUuid &id);

	virtual std::weak_ptr<IEffectProperty> GetGlobalProperty() override;
	virtual void ResetGlobalProperty()override;


	virtual std::vector<QUuid> GetResMeadiaIds(const QUuid& resId) override;

	virtual D2D1_RECT_D GetBoundingRectPts(const std::vector<QUuid>& vecId) override;

	// 判断当前媒体是否已经创建
	virtual bool IsMediaExist(const QUuid &uuid);
	virtual bool IsMediaExistByPlayerId(const QUuid &uuid);

	//创建/删除/清空媒体
	virtual	IMediaNode* CreateReplaceMedia(const QUuid & mediaId);																//..
	virtual	void DeleteMediaNode(const QUuid &id);
	virtual	void ClearMediaNodes();

	virtual	void AddMedia(std::shared_ptr<IMediaNode> ptr) override;
	virtual	std::shared_ptr<IMediaNode> CreateMediaNode() override;

	//Model数据保存与加载
	virtual void SaveMediaModelToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual void LoadMediaModelFromRapidJson(const rapidjson::Value &root) override;

private:
	std::shared_mutex	m_mtxMedia;
	std::map<QUuid,std::shared_ptr<IMediaNode>>	m_mapMediaNodes;	// 媒体集合
	std::vector<QUuid>m_vecMeidaNodes;								//维护媒体添加顺序，后添加的素材置于上层

	std::shared_ptr<IEffectProperty> m_pGlobalProperty = nullptr;	// 全局属性
};