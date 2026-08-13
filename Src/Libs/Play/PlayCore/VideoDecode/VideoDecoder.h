#pragma once
#include "AVDecoder.h"
#include <QStringList>

namespace PlayCore {
	typedef int VideoDecoderId;
	class CVideoDecoder : public CAVDecoder
	{
	public:
		CVideoDecoder();
		virtual ~CVideoDecoder();
		static QStringList supportedCodecs();
		static CVideoDecoder* create(VideoDecoderId id);
		std::string name() const;

		virtual VideoDecoderId id() const = 0;
		void setGpuIndex(unsigned int gpuIndex);

	public:
		int width;
		int  height;
		unsigned int m_gpuIndex = 0;
	};

	extern  VideoDecoderId VideoDecoderId_SW;
	extern  VideoDecoderId VideoDecoderId_CUDA;
	extern  VideoDecoderId VideoDecoderId_DXVA;
	extern  VideoDecoderId VideoDecoderId_D3D11;
}
