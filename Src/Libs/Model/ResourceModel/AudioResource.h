#pragma once
#include "ResourceBase.h"

class CAudioResource
	: public CResourceBase
	, public IAudioResource
{
	INIT_ICOPYINTERFACE(CAudioResource)
public:
public:
	CAudioResource();
	CAudioResource(const CAudioResource& other);
	CAudioResource& operator= (const CAudioResource& other);
	virtual ~CAudioResource();

	void SetSampleSize(int nVal) override;
	int GetSampleSize() override;

	void SetSampleRate(int nVal) override;
	int  GetSampleRate() override;

	void SetChannels(int nVal) override;
	int GetChannels() override;

protected:
	int m_nSampleSize;
	int m_nSampleRate;
	int m_nChannels;
};
