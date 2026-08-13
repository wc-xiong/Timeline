#pragma once

#include <QByteArray>
#include <QString>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cmath>
#include <QFile>
#include <QUuid>
#include <chrono>

using RjDocument = rapidjson::Document;
using RjValue = rapidjson::Value;
using RjAllocator = rapidjson::Document::AllocatorType;

inline double RapidJsonSafeDouble(double value)
{
	return std::isfinite(value) ? value : 0.0;
}

inline float RapidJsonSafeFloat(float value)
{
	return std::isfinite(value) ? value : 0.f;
}

inline QByteArray RapidJsonToUtf8Key(const QString& key)
{
	return key.toUtf8();
}

inline bool LoadRapidJsonFromFile(const QString& strPath, rapidjson::Document& doc)
{
	doc.SetObject();

	QFile file(strPath);
	if (!file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	const QByteArray data = file.readAll();
	file.close();
	if (data.isEmpty())
	{
		return false;
	}

	doc.Parse(data.constData(), static_cast<size_t>(data.size()));
	if (doc.HasParseError() || !doc.IsObject())
	{
		doc.SetObject();
		return false;
	}

	return true;
}

inline bool SaveRapidJsonToFile(const rapidjson::Value& obj, const QString& strPath)
{
	QFile file(strPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		assert(0);
		return false;
	}

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	if (!obj.Accept(writer))
	{
		file.close();
		assert(0);
		return false;
	}

	const qint64 written = file.write(buffer.GetString(), static_cast<qint64>(buffer.GetSize()));
	file.close();
	return written == static_cast<qint64>(buffer.GetSize());
}

inline int RapidJsonGetIntOrDefault(const rapidjson::Value& root, const char* pKey, int defaultValue = 0)
{
	if (!root.IsObject() || !root.HasMember(pKey))
		return defaultValue;
	const rapidjson::Value& v = root[pKey];
	if (v.IsInt())
		return v.GetInt();
	if (v.IsUint())
		return static_cast<int>(v.GetUint());
	return defaultValue;
}

inline bool RapidJsonGetBoolOrDefault(const rapidjson::Value& root, const char* pKey, bool defaultValue = false)
{
	if (!root.IsObject() || !root.HasMember(pKey) || !root[pKey].IsBool())
	{
		return defaultValue;
	}
	return root[pKey].GetBool();
}

inline double RapidJsonGetDoubleOrDefault(const rapidjson::Value& root, const char* pKey, double defaultValue = 0.0)
{
	if (!root.IsObject() || !root.HasMember(pKey) || !root[pKey].IsNumber())
	{
		return defaultValue;
	}
	return root[pKey].GetDouble();
}

inline QString RapidJsonGetStringOrDefault(const rapidjson::Value& root, const char* pKey, const QString& defaultValue = QString())
{
	if (!root.IsObject() || !root.HasMember(pKey) || !root[pKey].IsString())
	{
		return defaultValue;
	}
	return QString::fromUtf8(root[pKey].GetString());
}

inline bool RapidJsonValueToDouble(const rapidjson::Value& value, double& outValue)
{
	if (value.IsNumber())
	{
		outValue = value.GetDouble();
		return true;
	}

	if (value.IsString())
	{
		bool bOk = false;
		outValue = QString::fromUtf8(value.GetString(), value.GetStringLength()).toDouble(&bOk);
		return bOk;
	}

	return false;
}

inline const rapidjson::Value* RapidJsonFindMemberValue(const rapidjson::Value& object, const QString& key)
{
	if (!object.IsObject())
	{
		return nullptr;
	}

	const QByteArray keyUtf8 = key.toUtf8();
	if (!object.HasMember(keyUtf8.constData()))
	{
		return nullptr;
	}

	return &object[keyUtf8.constData()];
}

inline QString RapidJsonGetStringValue(const rapidjson::Value* pValue, const QString& defaultValue = QString())
{
	if (pValue == nullptr || !pValue->IsString())
	{
		return defaultValue;
	}

	return QString::fromUtf8(pValue->GetString(), static_cast<int>(pValue->GetStringLength()));
}

inline int RapidJsonGetIntValue(const rapidjson::Value* pValue, int defaultValue = 0)
{
	if (pValue == nullptr)
	{
		return defaultValue;
	}

	if (pValue->IsInt())
	{
		return pValue->GetInt();
	}

	if (pValue->IsString())
	{
		bool bOk = false;
		const QString intString = QString::fromUtf8(pValue->GetString(), static_cast<int>(pValue->GetStringLength()));
		const int intValue = intString.toInt(&bOk);
		if (bOk)
		{
			return intValue;
		}
	}

	return defaultValue;
}

inline quint32 JsonUIntMember(const rapidjson::Value& o, const char* key, quint32 defaultValue = 0)
{
	if (!o.IsObject() || !o.HasMember(key))
		return defaultValue;
	const rapidjson::Value& v = o[key];
	if (v.IsUint())
		return v.GetUint();
	if (v.IsInt())
		return static_cast<quint32>(v.GetInt());
	if (v.IsString())
	{
		bool ok = false;
		const QString s = QString::fromUtf8(v.GetString(), static_cast<int>(v.GetStringLength()));
		const unsigned u = s.toUInt(&ok);
		return ok ? static_cast<quint32>(u) : defaultValue;
	}
	return defaultValue;
}

inline int JsonIntMember(const rapidjson::Value& o, const char* key, int def)
{
	if (!o.IsObject() || !o.HasMember(key))
		return def;
	const rapidjson::Value& v = o[key];
	if (v.IsInt())
		return v.GetInt();
	if (v.IsUint())
		return static_cast<int>(v.GetUint());
	if (v.IsString())
	{
		bool ok = false;
		const int n = QString::fromUtf8(v.GetString(), static_cast<int>(v.GetStringLength())).toInt(&ok);
		return ok ? n : def;
	}
	return def;
}


inline void RapidJsonAddStringMember(rapidjson::Value& object,
	const QString& key,
	const QString& value,
	rapidjson::Document::AllocatorType& allocator)
{
	const QByteArray keyUtf8 = key.toUtf8();
	const QByteArray valueUtf8 = value.toUtf8();
	object.AddMember(
		rapidjson::Value(keyUtf8.constData(), allocator).Move(),
		rapidjson::Value(valueUtf8.constData(), allocator).Move(),
		allocator);
}

inline void RapidJsonAddIntMember(rapidjson::Value& object,
	const QString& key,
	int value,
	rapidjson::Document::AllocatorType& allocator)
{
	const QByteArray keyUtf8 = key.toUtf8();
	object.AddMember(rapidjson::Value(keyUtf8.constData(), allocator).Move(), value, allocator);
}


inline std::chrono::microseconds loadMicroseconds(const rapidjson::Value& obj, const char* key, std::chrono::microseconds defaultValue)
{
	if (!obj.HasMember(key))
	{
		return defaultValue;
	}

	const rapidjson::Value& v = obj[key];
	int64_t count = defaultValue.count();
	if (v.IsInt64())
	{
		count = v.GetInt64();
	}
	else if (v.IsUint64())
	{
		count = static_cast<int64_t>(v.GetUint64());
	}
	else if (v.IsInt())
	{
		count = v.GetInt();
	}
	else if (v.IsUint())
	{
		count = static_cast<int64_t>(v.GetUint());
	}
	else if (v.IsDouble())
	{
		count = static_cast<int64_t>(v.GetDouble());
	}
	else
	{
		return defaultValue;
	}

	return std::chrono::microseconds(count);
};

inline QString RjQString(const rapidjson::Value& o, const char* key)
{
	if (!o.IsObject() || !o.HasMember(key) || !o[key].IsString())
		return QString();
	const rapidjson::Value& v = o[key];
	return QString::fromUtf8(v.GetString(), static_cast<int>(v.GetStringLength()));
}

inline QUuid RjUuid(const rapidjson::Value& o, const char* key)
{
	const QString s = RjQString(o, key);
	if (s.isEmpty())
		return QUuid();
	return QUuid(s);
}

inline __int64 RjMsMember(const rapidjson::Value& o, const char* key, __int64 defVal)
{
	if (!o.IsObject() || !o.HasMember(key))
		return defVal;
	const rapidjson::Value& v = o[key];
	if (v.IsDouble())
		return static_cast<__int64>(v.GetDouble());
	if (v.IsInt64())
		return v.GetInt64();
	if (v.IsUint64())
		return static_cast<__int64>(v.GetUint64());
	if (v.IsInt())
		return v.GetInt();
	if (v.IsUint())
		return static_cast<__int64>(v.GetUint());
	return defVal;
}
