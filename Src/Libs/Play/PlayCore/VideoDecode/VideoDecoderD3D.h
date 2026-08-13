#pragma once

#include <unknwn.h>
#include <inttypes.h>
#include "VideoDecoderHW.h"
#include "VideoFormat.h"

namespace PlayCore {

    struct d3d_format_t
    {
        const char *name;
        int fourcc;
        CVideoFormat::PixelFormat pixfmt;
    };

    struct va_surface_t {
        va_surface_t() : ref(0), order(0) {}
        virtual ~va_surface_t() {}
        virtual void setSurface(IUnknown* s) = 0;
        virtual IUnknown* getSurface() const = 0;

        int ref;
        int order;
    };

    bool isIntelClearVideo(const GUID *guid);
    bool isNoEncrypt(const GUID* guid);
    int getSupportedFourcc(int *formats, UINT nb_formats);

    CVideoFormat::PixelFormat pixelFormatFromFourcc(int format);

    class CVideoDecoderD3D : public CVideoDecoderHW
    {
    public:
        CVideoDecoderD3D();
        virtual ~CVideoDecoderD3D();
        void setSurfaces(int num);
        int getSurfaces() const;
        virtual VideoDecoderId id() const = 0;

        bool openInternal() Q_DECL_OVERRIDE;
        void closeInternal() Q_DECL_OVERRIDE;
        void* setup(AVCodecContext *avctx) Q_DECL_OVERRIDE;
        bool getBuffer(void **opaque, uint8_t **data) Q_DECL_OVERRIDE;
        void releaseBuffer(void *opaque, uint8_t *data) Q_DECL_OVERRIDE;
        int aligned(int x);
        virtual bool setupSurfaceInterop() { return true; }
        virtual bool checkDevice() { return true; }

        virtual bool createDevice() = 0;
        virtual void destroyDevice() = 0;
        virtual std::vector<GUID> getSupportedCodecs() const = 0;
        virtual void* setupAVVAContext() = 0;
        virtual bool createDecoder(AVCodecID codec, int width, int height, std::vector<va_surface_t*>& surf) = 0;
        virtual void destroyDecoder() = 0;
        virtual int fourccFor(const GUID *guid) const = 0;
        const d3d_format_t* getD3DFormat(const AVCodecContext* avctx, const std::vector<GUID>& guids, GUID *selected) const;

    public:
        bool surface_auto;
        int surface_count;
        std::vector<IUnknown*> hw_surfaces;
        int format_fcc = 0;
        GUID codec_guid;

    private:
        int surface_width;
        int surface_height;
        unsigned surface_order;
        std::vector<va_surface_t*> surfaces;
    };

    template<typename T> int SelectConfig(AVCodecID codec_id, const T* cfgs, int nb_cfgs, T* cfg)
    {
       // std::cout << "Decoder configurations: " << nb_cfgs << std::endl;
        int cfg_score = 0;
        for (int i = 0; i < nb_cfgs; i++)
        {
            const T &c = cfgs[i];
//             std::cout << "configuration[" << i
//                 << "] ConfigBitstreamRaw " << c.ConfigBitstreamRaw
//                 << std::endl;
            int score = 0;
            if (1 == c.ConfigBitstreamRaw)
            {
                score = 1;
            }
            else if (AV_CODEC_ID_H264 == codec_id && 2 == c.ConfigBitstreamRaw)
            {
                score = 2;
            }
            else
            {
                continue;
            }

            if (isNoEncrypt(&c.guidConfigBitstreamEncryption))
            {
                score += 16;
            }
            if (cfg_score < score)
            {
                *cfg = c;
                cfg_score = score;
            }
        }
        if (cfg_score <= 0)
        {
            std::cout << "Failed to find a supported decoder configuration" << std::endl;
        }
        return cfg_score;
    }

#ifndef MAKEFOURCC //winrt
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
  ((DWORD)(BYTE)(ch0)|((DWORD)(BYTE)(ch1)<<8)|((DWORD)(BYTE)(ch2)<<16)|((DWORD)(BYTE)(ch3)<<24))
#endif //MAKEFOURCC
} //namespace PlayCore
