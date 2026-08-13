#pragma once

#include <rapidjson/document.h>

struct SDeviceMachineInfo
{
	float fCPU = 0.f;
	float fGPU = 0.f;
	float fMEM = 0.f;
	bool bDongleExist = false;
	bool bHasSyncCard = false;

	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		root.AddMember(rapidjson::StringRef("CPU"), static_cast<double>(fCPU), allocator);
		root.AddMember(rapidjson::StringRef("GPU"), static_cast<double>(fGPU), allocator);
		root.AddMember(rapidjson::StringRef("MEM"), static_cast<double>(fMEM), allocator);
		root.AddMember(rapidjson::StringRef("DongleState"), bDongleExist, allocator);
		root.AddMember(rapidjson::StringRef("HasSyncCard"), bHasSyncCard, allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& root)
	{
		if (!root.IsObject())
		{
			return false;
		}

		if (root.HasMember("CPU") && root["CPU"].IsNumber())
		{
			fCPU = static_cast<float>(root["CPU"].GetDouble());
		}
		if (root.HasMember("GPU") && root["GPU"].IsNumber())
		{
			fGPU = static_cast<float>(root["GPU"].GetDouble());
		}
		if (root.HasMember("MEM") && root["MEM"].IsNumber())
		{
			fMEM = static_cast<float>(root["MEM"].GetDouble());
		}
		if (root.HasMember("DongleState"))
		{
			bDongleExist = root["DongleState"].GetBool();
		}
		if (root.HasMember("HasSyncCard") && root["HasSyncCard"].IsBool())
		{
			bHasSyncCard = root["HasSyncCard"].GetBool();
		}
		return true;
	}
};
