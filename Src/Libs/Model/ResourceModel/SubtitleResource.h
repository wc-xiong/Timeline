#pragma once
#include "ResourceBase.h"
#include "Common/ResourceDefine.h"
#include <shared_mutex>

class CSubtitleResource
	: public CResourceBase
	, public ISubtitleResource
{
	INIT_ICOPYINTERFACE(CSubtitleResource)
public:
	CSubtitleResource();
	CSubtitleResource(const CSubtitleResource& other);
	CSubtitleResource& operator=(const CSubtitleResource& other);

	~CSubtitleResource();
public:
	virtual void SetSubtitleDatas(std::vector<SSubtitleDataNode> vecDatas) override;
	virtual std::vector<SSubtitleDataNode> GetSubtitleDatas() override;
	virtual bool GetSubTitleEmpty()override;
	SSubtitleDataNode GetIndexSubtitleNode(int nIndex)override;

	virtual int GetPlayTextIndexByTime(__int64 nMs) override;

	virtual void SetSubtitleDir(bool bHor) override {
		m_bShowHorDir = bHor;
	}
	virtual bool GetSubtitleDir() override {
		return m_bShowHorDir;
	}

	// 设置显示字体
	void SetShowFont(QFont font) override {
		m_textFont = font;
	}
	QFont GetShowFont() override {
		return m_textFont;
	}

	void SetFontColor(QColor clr) override {
		m_clrFont = clr;
	}
	QColor GetFontColor() override {
		return m_clrFont;
	}

	void SetTextBkClrEnable(bool bValue) override {
		m_bBkClrEnable = bValue;
	}
	bool GetTextBkClrEnable() override {
		return m_bBkClrEnable;
	}

	void SetTextBkClr(QColor clr) override {
		m_clrBk = clr;
	}
	QColor GetTextBkClr() override {
		return m_clrBk;
	}

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

private:
	std::shared_mutex m_mtxVecSubtitle;
	std::vector<SSubtitleDataNode> m_vecSubtitleDatas;	// 字幕数据

	bool m_bShowHorDir = true;	// 字幕显示方向，false为竖着	

	QFont m_textFont;	// 显示文本字体
	QColor m_clrFont;	// 显示文本颜色

	bool m_bBkClrEnable = false;	// 是否需要自定义背景色
	QColor m_clrBk;	// 背景色颜色
};

