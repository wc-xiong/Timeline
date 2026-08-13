#pragma once
#include <QUuid>
#include <unordered_map>
#include <unordered_set>
#include "CommonGlobal.h"

struct SUuidHashFunc
{
	size_t operator()(const QUuid& id) const noexcept
	{
		return qHash(id); // 直接使用Qt提供的qHash函数
	}
};

struct SUuidCompareFunc
{
	bool operator()(const QUuid& lhs, const QUuid& rhs) const noexcept
	{
		return lhs.toString() < rhs.toString();
	}
};

typedef std::unordered_set<QUuid, SUuidHashFunc> QUuidHash;

//namespace Uuid 
//{
//	enum EUuidType
//	{
//		UuidType_None,
//		UuidType_Resource = 1,
//		UuidType_ModelResource = 1 << 1,
//		UuidType_ModelData = 1 << 2,
//		UuidType_Slice = 1 << 3,
//		UuidType_Meida = 1 << 4,
//		UuidType_TimeLineObject = 1 << 5,
//		UuidType_Screen = 1 << 6,
//		UuidType_Player = 1 << 7,
//	};
//
//	std::unordered_map<QUuid, int, SUuidHashFunc>& Instanse()
//	{
//		static std::unordered_map<QUuid, int, SUuidHashFunc> Map;
//		return Map;
//	}
//
//	QUuid CreateUuid(int type)
//	{
//		const QUuid id = QUuid::createUuid();
//		Instanse().insert({ id,type });
//
//		return id;
//	}
//
//	void DeleteUuid(const QUuid& id)
//	{
//		Instanse().erase(id);
//	}
//
//	void AddUuidType(const QUuid& id, int type)
//	{
//		auto it = Instanse().find(id);
//		if (it != Instanse().end())
//			it->second |= type;
//		else
//			Instanse().insert({ id,type });
//	}
//
//	int GetUuidType(const QUuid& id)
//	{
//		auto it = Instanse().find(id);
//		if (it != Instanse().end())
//			return it->second;
//		else
//			return UuidType_None;
//	}
//}