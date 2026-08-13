#pragma once

#include "Common\IDemuxerInterface.h"
#include "DemuxerGlobal.h"
#include <memory>

class DEMUXER_EXPORT IBaseDemuxer
{
public:
	IBaseDemuxer() {};
	virtual ~IBaseDemuxer() {};

	virtual bool Demuxer(std::shared_ptr<SResFileDemuxInfo>& info) = 0;
};
