#pragma once
#include "UICommonGlobal.h"
#include <QHeaderView>
#include <vector>

// 目前此表头只适用横标题栏，且Model中表头数据一定返回空

class UICOMMON_EXPORT CHeaderViewEx : public QHeaderView
{
	Q_OBJECT

public:
	CHeaderViewEx(QWidget* pParent = 0);
	~CHeaderViewEx();

	void SetColLeftIntervals(const std::vector<int> &vec) {
		m_vecColLeftInterval = vec;
	}

	void SetColWidths(const std::vector<int> &vec) {
		m_vecColsWidth = vec;
	}

	void SetColNames(const QStringList &lstHeadColName) {
		m_lstHeadColName = lstHeadColName;
	}

	void SetTextLeftInterval(const int margin)
	{
		m_textLeftInterval = margin;
	}

	void SetTextBold(const bool b)
	{
		m_bBold = b;
	}

protected:
	virtual void paintEvent(QPaintEvent *event) override;

	virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
	QStringList m_lstHeadColName;	// 表头各列名称

	std::vector<int> m_vecColLeftInterval;	// 表头各列文字左边距
	
	std::vector<int> m_vecColsWidth;	// 表头每列宽度

	int	m_textLeftInterval;

	bool m_bBold = true;
};
