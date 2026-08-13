#pragma once
#include "IXmlInterface.h"

QString s_strVersion;

namespace IPrjInterface
{

	//void SetVersion(const QString& version)
	//{
	//	s_strVersion = version;
	//}

	QString GetVersion()
	{
		if (s_strVersion == "V.Test")
			return s_strVersion;

		QString str = s_strVersion.left(s_strVersion.indexOf('(')).right(s_strVersion.indexOf(':') - 1);
		int a = str.toInt();

		if (a <= 22436)
			return "0.7.0.0";
		else if (a > 22436 && a <= 24832)
			return "1.0.0.0";
		else if (a > 24832 && a <= 26819)
			return "1.2.0.0";
		else if (a > 26819 && a <= 28432)
			return "2.0.0.0";
		else if (a > 28432 && a <= 29819)
			return "2.2.0.0";
		else
			return "2.5.0.0";
	}

	QString GetVersion(const QString& version)
	{
		if (version == "V.Test")
			return version;

		QString str = version.left(version.indexOf('(')).right(version.indexOf(':') - 1);
		int a = str.toInt();

		if (a <= 22436)
			return "0.7.0.0";
		else if (a > 22436 && a <= 24832)
			return "1.0.0.0";
		else if (a > 24832 && a <= 26819)
			return "1.2.0.0";
		else if (a > 26819 && a <= 28432)
			return "2.0.0.0";
		else if (a > 28432 && a <= 29819)
			return "2.2.0.0";
		else
			return "2.5.0.0";
	}
}