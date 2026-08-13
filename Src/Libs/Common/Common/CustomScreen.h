#pragma once

#include <QVariant>
#include <QUuid>

struct SCustomScreen 
{
	SCustomScreen(quint32 _physicalId, QUuid _id, QString _name, bool _bVirtual)
	{
		physicalId = _physicalId;
		id = _id;
		name = _name;
		bVirtual = _bVirtual;
	}

	SCustomScreen()
	{
		physicalId = 0;
		id = QUuid();
		name = "";
		bVirtual = false;
	}

	bool Equal(const SCustomScreen& other)
	{
		if (bVirtual)
		{
			return id == other.id;
		}

		return physicalId == other.physicalId;
	}

	quint32 physicalId = 0;
	QUuid id = QUuid();
	QString name;
	bool bVirtual = false;
};

Q_DECLARE_METATYPE(SCustomScreen)