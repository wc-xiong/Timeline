#pragma once
#include <QPointF>
#include <QSizeF>
#include <QVector>
#include <QMatrix4x4>
#include <QVector2D>

class CTransformHelper
{
public:
	CTransformHelper(const QPointF& pos, const QSizeF& size, float angle);
	~CTransformHelper();

	void Move(float xoffset, float yoffset);
	void Scale(float xradio, float yradio, float centerx, float centery);
	void Scale(float xradio, float yradio);
	void Rotate(float angleoffset, float centerx, float centery);
	void Rotate(float angleoffset);

	QPointF getPos();
	QSizeF	getSize();
	float	getAngle();

private:
	void calculate();

	QVector<QPointF> m_vecPoints;//点的顺序，x轴向右，y轴向上，顺时针顺序
	float m_angle;
	QPointF m_pos;
	QSizeF m_size;
};