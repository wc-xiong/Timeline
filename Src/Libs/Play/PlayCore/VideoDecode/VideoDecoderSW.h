#include "VideoDecoder.h"

namespace PlayCore
{
	class CVideoDecoderSW : public CVideoDecoder
	{
	public:
		CVideoDecoderSW();
		VideoDecoderId id() const Q_DECL_OVERRIDE Q_DECL_FINAL;
		virtual bool openInternal();
	};
}
