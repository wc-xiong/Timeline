#pragma once
#include "ResourceBase.h"
#include "ResourceModelGlobal.h"
#include "Common/ResourceDefine.h"
#include "TurnPageResource.h"

#include <QReadWriteLock>

class CTeleprompterResource
	: public CResourceBase 
	, public ITeleprompterResource
	, public CTurnPageResource
{
	INIT_ICOPYINTERFACE(CTeleprompterResource)
public:
	CTeleprompterResource();
	CTeleprompterResource(const CTeleprompterResource& other);

	~CTeleprompterResource();
public:
	void AddDefaultTeleprompterItem(int index) override;
	bool DeleteTeleprompterItem(int index) override;

	void SetTeleprompterItemDur(int index, long long  duration) override;
	void SetTeleprompterItemText(int index, const QString &text) override;

	void SetTeleprompterData(std::vector <STeleprompterItem> itemList) override;
	std::vector <STeleprompterItem> GetTeleprompterData() override;

	long long GetTeleprompterItemDur(int index) override;
	QString GetTeleprompterItemText(int index) override;

	int GetTeleprompterItemCount() override {
		return (int)m_vecTeleprompterDatas.size();
	}

	// 设置显示字体
	void SetShowFont(QFont font) override {
		m_textFont = font;
	}

	QFont GetShowFont() override {
		return m_textFont;
	}

	// 文本对齐方式
	void SetAlignment(int nAlignment) override {
		m_nAlignment = nAlignment;
	};
	int GetAlignment()  override {
		return m_nAlignment;
	};

	// 字体颜色
	virtual void SetFontColor(QColor color) override {
		m_clrFont = color;
	}
	virtual QColor GetFontColor() override {
		return m_clrFont;
	}

	// 字幕显示方向
	virtual void SetShowDir(bool bHor) {
		m_bShowHorDir = bHor;
	};
	virtual bool GetShowDir() {
		return m_bShowHorDir;
	};

	// 背景颜色
	virtual void SetTextBkClrEnable(bool bValue)override {
		m_bBkClrEnable = bValue;
	}
	virtual bool GetTextBkClrEnable()override {
		return m_bBkClrEnable;
	}

	virtual void SetTextBkClr(QColor color)override {
		m_clrBk = color;
	}
	virtual QColor GetTextBkClr()override {
		return m_clrBk;
	}

	void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

private:
	std::vector<STeleprompterItem> m_vecTeleprompterDatas = std::vector<STeleprompterItem>(8);
	bool m_bShowHorDir = true;
	QFont m_textFont;	// 显示文本字体
	QColor m_clrFont;	// 显示文本颜色
	int m_nAlignment = Qt::AlignLeft;    //文本对齐方式
	bool m_bBkClrEnable = false;	// 是否需要自定义背景色
	QColor m_clrBk;	// 背景色颜色
};

