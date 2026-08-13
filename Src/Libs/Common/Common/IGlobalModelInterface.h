#pragma once
#include "CommonGlobal.h"
#include <list>
#include <QUuid>
#include <QRect>
#include <QString>
#include <QDomNode>
#include <QDateTime>
#include <rapidjson/document.h>

enum SoftwareProjectType
{
	ESPT_Normal,
	ESPT_Pack,
	ESPT_SaveAs,
	ESPT_Temp,
	ESPT_SaveDebugSnapshot,	//调试数据快照保存
};

enum ProjectLoadResult
{
	EPLR_Correct,			//成功
	EPLR_VersionError,		//版本不一致
	EPLR_Error,				//未定义错误
};

struct SRecentPrjNode
{
	QString strPath;
	QUuid prjId;
	SRecentPrjNode() {}
	SRecentPrjNode(const QString &path, const QUuid &id) :
		strPath(path), prjId(id) {}
};

struct SPrjJsonNodeData
{
	rapidjson::Document ResourceModelData;
	rapidjson::Document MediaModelData;
	rapidjson::Document ModelModelData;
	rapidjson::Document SliceModelData;
	rapidjson::Document TimelineModelData;
	rapidjson::Document virtualScreenData;
	rapidjson::Document TimeCodeRecvData;
	SPrjJsonNodeData()
	{
		ResourceModelData.SetObject();
		MediaModelData.SetObject();
		ModelModelData.SetObject();
		SliceModelData.SetObject();
		TimelineModelData.SetObject();
		virtualScreenData.SetObject();
		TimeCodeRecvData.SetObject();
	}
	SPrjJsonNodeData(const rapidjson::Value& res,
		const rapidjson::Value& media,
		const rapidjson::Value& model,
		const rapidjson::Value& slice,
		const rapidjson::Value& timeline,
		const rapidjson::Value& screenData,
		const rapidjson::Value& timeCodeRecvData
	)
	{
		ResourceModelData.SetObject();
		ResourceModelData.CopyFrom(res, ResourceModelData.GetAllocator());
		MediaModelData.SetObject();
		MediaModelData.CopyFrom(media, MediaModelData.GetAllocator());
		ModelModelData.SetObject();
		ModelModelData.CopyFrom(model, ModelModelData.GetAllocator());
		SliceModelData.SetObject();
		SliceModelData.CopyFrom(slice, SliceModelData.GetAllocator());
		TimelineModelData.SetObject();
		TimelineModelData.CopyFrom(timeline, TimelineModelData.GetAllocator());
		virtualScreenData.SetObject();
		virtualScreenData.CopyFrom(screenData, virtualScreenData.GetAllocator());
		TimeCodeRecvData.SetObject();
		TimeCodeRecvData.CopyFrom(timeCodeRecvData, TimeCodeRecvData.GetAllocator());
	}
	SPrjJsonNodeData(const SPrjJsonNodeData& other)
	{
		ResourceModelData.SetObject();
		ResourceModelData.CopyFrom(other.ResourceModelData, ResourceModelData.GetAllocator());
		MediaModelData.SetObject();
		MediaModelData.CopyFrom(other.MediaModelData, MediaModelData.GetAllocator());
		ModelModelData.SetObject();
		ModelModelData.CopyFrom(other.ModelModelData, ModelModelData.GetAllocator());
		SliceModelData.SetObject();
		SliceModelData.CopyFrom(other.SliceModelData, SliceModelData.GetAllocator());
		TimelineModelData.SetObject();
		TimelineModelData.CopyFrom(other.TimelineModelData, TimelineModelData.GetAllocator());
		virtualScreenData.SetObject();
		virtualScreenData.CopyFrom(other.virtualScreenData, virtualScreenData.GetAllocator());
		TimeCodeRecvData.SetObject();
		TimeCodeRecvData.CopyFrom(other.TimeCodeRecvData, TimeCodeRecvData.GetAllocator());
	}
	SPrjJsonNodeData& operator=(const SPrjJsonNodeData& other)
	{
		if (this == &other)
		{
			return *this;
		}
		ResourceModelData.SetObject();
		ResourceModelData.CopyFrom(other.ResourceModelData, ResourceModelData.GetAllocator());
		MediaModelData.SetObject();
		MediaModelData.CopyFrom(other.MediaModelData, MediaModelData.GetAllocator());
		ModelModelData.SetObject();
		ModelModelData.CopyFrom(other.ModelModelData, ModelModelData.GetAllocator());
		SliceModelData.SetObject();
		SliceModelData.CopyFrom(other.SliceModelData, SliceModelData.GetAllocator());
		TimelineModelData.SetObject();
		TimelineModelData.CopyFrom(other.TimelineModelData, TimelineModelData.GetAllocator());
		virtualScreenData.SetObject();
		virtualScreenData.CopyFrom(other.virtualScreenData, virtualScreenData.GetAllocator());
		TimeCodeRecvData.SetObject();
		TimeCodeRecvData.CopyFrom(other.TimeCodeRecvData, TimeCodeRecvData.GetAllocator());
		return *this;
	}
};

// 操作日志记录，单条
struct SOPRecord
{
	unsigned int m_index;		// 读入数据序号
	QString		m_time;			// 时间
	QString		m_module;		// 模块
	QString		m_function;		// 功能
	QString		m_context;		// 内容
	int			m_controller;	// 操作端
	SOPRecord() : m_index(0), m_time("")
		, m_module(""), m_function("")
		, m_context(""), m_controller(-1)
	{}
	SOPRecord(const unsigned int&index, const QString& time, const QString& smodule,
		const QString& func, const QString& context, int controller)
		: m_index(index), m_time(time), m_module(smodule), m_function(func), m_context(context), m_controller(controller)
	{}
	SOPRecord(const SOPRecord& record)
		: m_index(record.m_index), m_time(record.m_time), m_module(record.m_module)
		, m_function(record.m_function), m_context(record.m_context), m_controller(record.m_controller)
	{}
	bool operator < (const SOPRecord& record) const
	{
		if (this->m_time == record.m_time)
			return this->m_index < record.m_index;
		return this->m_time < record.m_time;
	}
	bool operator > (const SOPRecord& record) const
	{
		if (this->m_time == record.m_time)
			return this->m_index > record.m_index;
		return this->m_time > record.m_time;
	}
};
