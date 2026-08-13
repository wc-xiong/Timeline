#pragma once
#include <QWidget>
#include <QUuid>

class CClipWidget : public QWidget
{
	Q_OBJECT
public:
	CClipWidget(QWidget* pParent = 0);
	~CClipWidget();

	void SetObjectId(const QUuid& id);
	void UpdateView();

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QUuid m_objId;
};