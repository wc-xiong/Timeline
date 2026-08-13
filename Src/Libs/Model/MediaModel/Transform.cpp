#include "Transform.h"
CTransformHelper::CTransformHelper(const QPointF& pos, const QSizeF& size, float angle)
{
	m_angle = angle;
	m_pos = pos;
	m_size = size;

	m_vecPoints.clear();
	QPointF pt = QPointF(pos.x(), pos.y());
	m_vecPoints.push_back(pt);
	pt = QPointF(pos.x(), pos.y() + size.height());
	m_vecPoints.push_back(pt);
	pt = QPointF(pos.x() + size.width(), pos.y() + size.height());
	m_vecPoints.push_back(pt);
	pt = QPointF(pos.x() + size.width(), pos.y());
	m_vecPoints.push_back(pt);

	QMatrix4x4 temp;
	temp.translate(pos.x(), pos.y());
	temp.rotate(angle, 0, 0, 1);
	temp.translate(-pos.x(), -pos.y());

	for (auto& pt : m_vecPoints)
		pt = temp.map(pt);
}

CTransformHelper::~CTransformHelper()
{
}

void CTransformHelper::Move(float xoffset, float yoffset)
{
	QMatrix4x4 temp;
	temp.translate(xoffset, yoffset);
	for (auto& pt : m_vecPoints)
		pt = temp.map(pt);

	m_pos = m_vecPoints[0];
	m_size = QSizeF(QVector2D(m_vecPoints[0] - m_vecPoints[3]).length(),
		QVector2D(m_vecPoints[0] - m_vecPoints[1]).length());
}

void CTransformHelper::Scale(float xradio, float yradio, float centerx, float centery)
{
	QMatrix4x4 temp;
	temp.translate(centerx, centery);
	temp.rotate(m_angle, 0, 0, 1);
	temp.scale(xradio, yradio);
	temp.rotate(-m_angle, 0, 0, 1);
	temp.translate(-centerx, -centery);

	for (auto& pt : m_vecPoints)
		pt = temp.map(pt);

	m_pos = m_vecPoints[0];
	if (xradio < 0)m_pos = m_vecPoints[3];
	if (yradio < 0)m_pos = m_vecPoints[1];
	if (xradio < 0 && yradio < 0) m_pos = m_vecPoints[2];
	m_size = QSizeF(QVector2D(m_vecPoints[0] - m_vecPoints[3]).length(),
		QVector2D(m_vecPoints[0] - m_vecPoints[1]).length());
}

void CTransformHelper::Scale(float xradio, float yradio)
{
	Scale(xradio, yradio, m_vecPoints[0].x(), m_vecPoints[0].y());
}

void CTransformHelper::Rotate(float angleoffset, float centerx, float centery)
{
	m_angle += angleoffset;
	if (m_angle <= 0) m_angle += 360;
	if (m_angle >= 360) m_angle -= 360;

	QMatrix4x4 temp;
	temp.translate(centerx, centery);
	temp.rotate(angleoffset, 0, 0, 1);
	temp.translate(-centerx, -centery);
	for (auto& pt : m_vecPoints)
		pt = temp.map(pt);

	m_pos = m_vecPoints[0];
	m_size = QSizeF(QVector2D(m_vecPoints[0] - m_vecPoints[3]).length(),
		QVector2D(m_vecPoints[0] - m_vecPoints[1]).length());
}

void CTransformHelper::Rotate(float angleoffset)
{
	auto center = (m_vecPoints[0] + m_vecPoints[2]) / 2;
	Rotate(angleoffset, center.x(), center.y());
}

void CTransformHelper::calculate()
{
	m_pos = m_vecPoints[0];
	m_size = QSizeF(QVector2D(m_vecPoints[0] - m_vecPoints[3]).length(),
		QVector2D(m_vecPoints[0] - m_vecPoints[1]).length());
}

QPointF CTransformHelper::getPos()
{
	return m_pos;
}

QSizeF CTransformHelper::getSize()
{
	return m_size;
}

float CTransformHelper::getAngle()
{
	return m_angle;
}
