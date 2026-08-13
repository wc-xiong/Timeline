#include "AudioResource.h"
#include "UICommon/UICommonSizeDefine.h"
#include "Common/ResourceDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CAudioResource::CAudioResource()
	: m_nSampleRate(0)
	, m_nChannels(0)
	, m_nSampleSize(0)
{
	m_nResType = ERT_Audio;
	m_bDemuxered = false;

	m_szResolution = QSize(1920, 1080);
}

CAudioResource::CAudioResource(const CAudioResource& other)
	: CResourceBase(other)
{
	m_nSampleSize = other.m_nSampleSize;
	m_nSampleRate = other.m_nSampleRate;
	m_nChannels = other.m_nChannels;
}

CAudioResource& CAudioResource::operator=(const CAudioResource& other)
{
	*((CResourceBase*)this) = other;

	return *this;
}

CAudioResource::~CAudioResource()
{

}

void CAudioResource::SetSampleSize(int nVal)
{
	m_nSampleSize = nVal;
}

int CAudioResource::GetSampleSize()
{
	return m_nSampleSize;
}

void CAudioResource::SetSampleRate(int nVal)
{
	m_nSampleRate = nVal;
}

int CAudioResource::GetSampleRate()
{
	return m_nSampleRate;
}

void CAudioResource::SetChannels(int nVal)
{
	m_nChannels = nVal;
}

int CAudioResource::GetChannels()
{
	return m_nChannels;
}
