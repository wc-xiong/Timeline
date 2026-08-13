#include "VideoFormat.h"
#include <cmath>
#include <QtCore/QVector>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define PIXFMT_NE(B, L) CVideoFormat::Format_##L

//#define DESC_VAL(X) (X##_minus1 + 1)
#define DESC_VAL(X) (X## + 1)
namespace PlayCore {

    // TODO: use FFmpeg macros to get right endian
    static const struct {
        CVideoFormat::PixelFormat fmt;
        AVPixelFormat ff; //int
    } pixfmt_map[] = {
        { CVideoFormat::Format_YUV420P, AV_PIX_FMT_YUV420P },   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
        { CVideoFormat::Format_YV12, AV_PIX_FMT_YUV420P },   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
        { CVideoFormat::Format_YUYV, AV_PIX_FMT_YUYV422 }, //??   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
        { CVideoFormat::Format_RGB24, AV_PIX_FMT_RGB24 },     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
        { CVideoFormat::Format_BGR24, AV_PIX_FMT_BGR24 },     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
        { CVideoFormat::Format_YUV422P, AV_PIX_FMT_YUV422P},   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
        { CVideoFormat::Format_YUV444P, AV_PIX_FMT_YUV444P },   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
        { CVideoFormat::Format_YUV410P, AV_PIX_FMT_YUV410P },   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
        { CVideoFormat::Format_YUV411P, AV_PIX_FMT_YUV411P },   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
        { CVideoFormat::Format_Y8, AV_PIX_FMT_GRAY8 },     ///<        Y        ,  8bpp

        { CVideoFormat::Format_YUV420P, AV_PIX_FMT_YUVJ420P },  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV420P and setting color_range

        { CVideoFormat::Format_UYVY, AV_PIX_FMT_UYVY422 },   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1

        { CVideoFormat::Format_NV12, AV_PIX_FMT_NV12 },      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
        { CVideoFormat::Format_NV21, AV_PIX_FMT_NV21 },      ///< as above, but U and V bytes are swapped
        { CVideoFormat::Format_ARGB32, AV_PIX_FMT_ARGB },      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
        { CVideoFormat::Format_RGBA32, AV_PIX_FMT_RGBA },      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
        { CVideoFormat::Format_ABGR32, AV_PIX_FMT_ABGR },      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
        { CVideoFormat::Format_BGRA32, AV_PIX_FMT_BGRA },      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
        { CVideoFormat::Format_Y16, AV_PIX_FMT_GRAY16LE },  ///<        Y        , 16bpp, little-endian
        { CVideoFormat::Format_RGB48BE, AV_PIX_FMT_RGB48BE },   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
        { CVideoFormat::Format_RGB48LE, AV_PIX_FMT_RGB48LE },   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian
        { CVideoFormat::Format_RGB565, AV_PIX_FMT_RGB565 },  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), native-endian
        { CVideoFormat::Format_RGB555, AV_PIX_FMT_RGB555 },  ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), native-endian, be: most significant bit to 1
        { CVideoFormat::Format_BGR565, AV_PIX_FMT_BGR565 },  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), native-endian
        { CVideoFormat::Format_BGR555, AV_PIX_FMT_BGR555 },  ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), native-endian, be: most significant bit to 1

        { CVideoFormat::Format_BGR48BE, AV_PIX_FMT_BGR48BE },   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
        { CVideoFormat::Format_BGR48LE, AV_PIX_FMT_BGR48LE },   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

        { CVideoFormat::Format_YUV420P9BE, AV_PIX_FMT_YUV420P9BE }, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
        { CVideoFormat::Format_YUV420P9LE, AV_PIX_FMT_YUV420P9LE }, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
        { CVideoFormat::Format_YUV420P10BE, AV_PIX_FMT_YUV420P10BE },///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
        { CVideoFormat::Format_YUV420P10LE, AV_PIX_FMT_YUV420P10LE },///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
        { CVideoFormat::Format_YUV422P10BE, AV_PIX_FMT_YUV422P10BE },///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
        { CVideoFormat::Format_YUV422P10LE, AV_PIX_FMT_YUV422P10LE },///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
        { CVideoFormat::Format_YUV444P9BE, AV_PIX_FMT_YUV444P9BE }, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
        { CVideoFormat::Format_YUV444P9LE, AV_PIX_FMT_YUV444P9LE }, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
        { CVideoFormat::Format_YUV444P10BE, AV_PIX_FMT_YUV444P10BE },///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
        { CVideoFormat::Format_YUV444P10LE, AV_PIX_FMT_YUV444P10LE },///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
        { CVideoFormat::Format_YUV422P9BE, AV_PIX_FMT_YUV422P9BE }, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
        { CVideoFormat::Format_YUV422P9LE, AV_PIX_FMT_YUV422P9LE }, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
        { CVideoFormat::Format_RGB32, AV_PIX_FMT_RGB32 }, //auto endian
        { CVideoFormat::Format_BGR32, AV_PIX_FMT_BGR32 }, //auto endian
        { CVideoFormat::Format_RGB48, AV_PIX_FMT_RGB48 },   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
        { CVideoFormat::Format_BGR48, AV_PIX_FMT_BGR48 },   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian

        { CVideoFormat::Format_VYUY, AV_PIX_FMT_UYVY422 }, // FIXME: hack for invalid ffmpeg formats

        { CVideoFormat::Format_VYU, AV_PIX_FMT_RGB32 },
    #ifdef AV_PIX_FMT_XYZ12
        { CVideoFormat::Format_XYZ12, AV_PIX_FMT_XYZ12 },
        { CVideoFormat::Format_XYZ12LE, AV_PIX_FMT_XYZ12LE },
        { CVideoFormat::Format_XYZ12BE, AV_PIX_FMT_XYZ12BE },
    #endif
        { CVideoFormat::Format_Invalid, AV_PIX_FMT_NONE },
    };

    CVideoFormat::PixelFormat CVideoFormat::pixelFormatFromFFmpeg(int ff)
    {
        for (unsigned int i = 0; i < sizeof(pixfmt_map) / sizeof(pixfmt_map[0]); ++i)
        {
            if (pixfmt_map[i].ff == ff)
            {
                return pixfmt_map[i].fmt;
            }
        }
        return CVideoFormat::Format_Invalid;
    }

    int CVideoFormat::pixelFormatToFFmpeg(CVideoFormat::PixelFormat fmt)
    {
        for (unsigned int i = 0; i < sizeof(pixfmt_map) / sizeof(pixfmt_map[0]); ++i)
        {
            if (pixfmt_map[i].fmt == fmt)
            {
                return pixfmt_map[i].ff;
            }
        }
        return AV_PIX_FMT_NONE;
    }

    std::vector<int> CVideoFormat::pixelFormatsFFmpeg()
    {
        static std::vector<int> sFmts;
        if (sFmts.empty())
        {
            const AVPixFmtDescriptor *desc = NULL;
            while ((desc = av_pix_fmt_desc_next(desc)))
            {
                if ((desc->flags & AV_PIX_FMT_FLAG_HWACCEL) == AV_PIX_FMT_FLAG_HWACCEL)
                {
                    continue;
                }
                sFmts.push_back(av_pix_fmt_desc_get_id(desc));
            }
        }
        return sFmts;
    }

    /*!
        Returns a video pixel format equivalent to an image \a format.  If there is no equivalent
        format CVideoFormat::InvalidType is returned instead.

        \note In general \l QImage does not handle YUV formats.

    */
    static const struct {
        CVideoFormat::PixelFormat fmt;
        QImage::Format qfmt;
    } qpixfmt_map[] = {
        { CVideoFormat::Format_RGB32, QImage::Format_ARGB32 },
        { CVideoFormat::Format_RGB32, QImage::Format_RGB32 },
        { CVideoFormat::Format_RGBA32, QImage::Format_RGBA8888 }, //be 0xRRGGBBAA, le 0xAABBGGRR
        { CVideoFormat::Format_RGB565, QImage::Format_RGB16 },
        { CVideoFormat::Format_BGR565, (QImage::Format) - QImage::Format_RGB16 },
        { CVideoFormat::Format_RGB555, QImage::Format_RGB555 },
        { CVideoFormat::Format_BGR555, (QImage::Format) - QImage::Format_RGB555 },
        { CVideoFormat::Format_RGB24, QImage::Format_RGB888 },
        { CVideoFormat::Format_BGR24, (QImage::Format) - QImage::Format_RGB888 },
        { CVideoFormat::Format_Invalid, QImage::Format_Invalid }
    };

    CVideoFormat::PixelFormat CVideoFormat::pixelFormatFromImageFormat(QImage::Format format)
    {
        for (int i = 0; qpixfmt_map[i].fmt != Format_Invalid; ++i)
        {
            if (qpixfmt_map[i].qfmt == format)
            {
                return qpixfmt_map[i].fmt;
            }
        }
        return Format_Invalid;
    }

    QImage::Format CVideoFormat::imageFormatFromPixelFormat(PixelFormat format)
    {
        for (int i = 0; qpixfmt_map[i].fmt != Format_Invalid; ++i)
        {
            if (qpixfmt_map[i].fmt == format)
            {
                return qpixfmt_map[i].qfmt;
            }
        }
        return QImage::Format_Invalid;
    }


    CVideoFormat::CVideoFormat(PixelFormat format)
        :m_pixfmt(format)
        , m_pixfmt_ff(AV_PIX_FMT_NONE)
        , m_qpixfmt(QImage::Format_Invalid)
        , m_planes(0)
        , m_bpp(0)
        , m_bpp_pad(0)
        , m_bpc(0)
        , m_pixdesc(0)
    {
        if (format == CVideoFormat::Format_Invalid) {
            m_pixfmt_ff = AV_PIX_FMT_NONE;
            m_qpixfmt = QImage::Format_Invalid;
            return;
        }
        initInternal(format);
    }

    CVideoFormat::~CVideoFormat()
    {

    }

    bool CVideoFormat::operator==(const CVideoFormat &other) const
    {
        return m_pixfmt_ff == other.m_pixfmt_ff;
    }

    bool CVideoFormat::operator==(CVideoFormat::PixelFormat fmt) const
    {
        return m_pixfmt == fmt;
    }

    bool CVideoFormat::operator==(QImage::Format qpixfmt) const
    {
        return m_qpixfmt == qpixfmt;
    }

    bool CVideoFormat::operator==(int fffmt) const
    {
        return m_pixfmt_ff == fffmt;
    }

    bool CVideoFormat::operator!=(const CVideoFormat& other) const
    {
        return !(*this == other);
    }

    bool CVideoFormat::operator!=(CVideoFormat::PixelFormat fmt) const
    {
        return m_pixfmt != fmt;
    }

    bool CVideoFormat::operator!=(QImage::Format qpixfmt) const
    {
        return m_qpixfmt != qpixfmt;
    }

    bool CVideoFormat::operator!=(int fffmt) const
    {
        return m_pixfmt_ff != fffmt;
    }

    bool CVideoFormat::isValid() const
    {
        return m_pixfmt_ff != AV_PIX_FMT_NONE || m_pixfmt != Format_Invalid;
    }

    CVideoFormat::PixelFormat CVideoFormat::pixelFormat() const
    {
        return m_pixfmt;
    }

    int CVideoFormat::pixelFormatFFmpeg() const
    {
        return m_pixfmt_ff;
    }

    QImage::Format CVideoFormat::imageFormat() const
    {
        return m_qpixfmt;
    }

    std::string CVideoFormat::name() const
    {
        return nameInternal();
    }

    void CVideoFormat::setPixelFormat(PixelFormat format)
    {
        m_pixfmt = format;
        initInternal(format);
    }

    void CVideoFormat::setPixelFormatFFmpeg(int format)
    {
        m_pixfmt_ff = (AVPixelFormat)format;
        initInternal((AVPixelFormat)format);
    }

    int CVideoFormat::channels() const
    {
        if (!m_pixdesc)
        {
            return 0;
        }
        return m_pixdesc->nb_components;
    }

    int CVideoFormat::channels(int plane) const
    {
        if (plane > m_channels.size())
        {
            return 0;
        }
        return m_channels[plane];
    }

    int CVideoFormat::planeCount() const
    {
        return m_planes;
    }

    int CVideoFormat::bitsPerPixel() const
    {
        return m_bpp;
    }

    int CVideoFormat::bitsPerPixelPadded() const
    {
        return m_bpp_pad;
    }

    int CVideoFormat::bitsPerPixel(int plane) const
    {
        //must be a valid index position in the vector
        if (plane >= m_bpps.size())
        {
            return 0;
        }
        return m_bpps[plane];
    }

    int CVideoFormat::bytesPerPixel() const
    {
        return (bitsPerPixel() + 7) >> 3;
    }

    int CVideoFormat::bytesPerPixel(int plane) const
    {
        return (bitsPerPixel(plane) + 7) >> 3;
    }

    int CVideoFormat::bitsPerComponent() const
    {
        return m_bpc;
    }

    int CVideoFormat::bytesPerLine(int width, int plane) const
    {
        return bytesPerLineInternal(width, plane);
    }

    int CVideoFormat::chromaWidth(int lumaWidth) const
    {
        return -((-lumaWidth) >> m_pixdesc->log2_chroma_w);
    }

    int CVideoFormat::chromaHeight(int lumaHeight) const
    {
        return -((-lumaHeight) >> m_pixdesc->log2_chroma_h);
    }

    int CVideoFormat::width(int lumaWidth, int plane) const
    {
        if (plane <= 0)
        {
            return lumaWidth;
        }
        return chromaWidth(lumaWidth);
    }

    int CVideoFormat::height(int lumaHeight, int plane) const
    {
        if (plane <= 0)
        {
            return lumaHeight;
        }
        return chromaHeight(lumaHeight);
    }

    double CVideoFormat::normalizedWidth(int plane) const
    {
        if (plane <= 0)
        {
            return 1.0;
        }
        return 1.0 / std::pow(2.0, double(m_pixdesc->log2_chroma_w));
    }

    double CVideoFormat::normalizedHeight(int plane) const
    {
        if (plane <= 0)
        {
            return 1.0;
        }
        return 1.0 / std::pow(2.0, double(m_pixdesc->log2_chroma_h));
    }

    // test AV_PIX_FMT_FLAG_XXX
    bool CVideoFormat::isBigEndian() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_BE) == AV_PIX_FMT_FLAG_BE;
    }

    bool CVideoFormat::hasPalette() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_PAL) == AV_PIX_FMT_FLAG_PAL;
    }

    //bool CVideoFormat::isPseudoPaletted() const
    //{
        //return (flagsInternal() & AV_PIX_FMT_FLAG_PSEUDOPAL) == AV_PIX_FMT_FLAG_PSEUDOPAL;
    //}

    bool CVideoFormat::isBitStream() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_BITSTREAM) == AV_PIX_FMT_FLAG_BITSTREAM;
    }

    bool CVideoFormat::isHWAccelerated() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_HWACCEL) == AV_PIX_FMT_FLAG_HWACCEL;
    }

    bool CVideoFormat::isPlanar() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_PLANAR) == AV_PIX_FMT_FLAG_PLANAR;
    }

    bool CVideoFormat::isRGB() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_RGB) == AV_PIX_FMT_FLAG_RGB && m_pixfmt != Format_VYU;
    }

    bool CVideoFormat::isXYZ() const
    {
        return m_pixfmt == Format_XYZ12 || m_pixfmt == Format_XYZ12LE || m_pixfmt == Format_XYZ12BE;
    }

    bool CVideoFormat::hasAlpha() const
    {
        return (flagsInternal() & AV_PIX_FMT_FLAG_ALPHA) == AV_PIX_FMT_FLAG_ALPHA;
    }

    bool CVideoFormat::isPlanar(PixelFormat pixfmt)
    {
        return pixfmt == Format_YUV420P || pixfmt == Format_NV12 || pixfmt == Format_NV21 || pixfmt == Format_YV12
            || pixfmt == Format_YUV410P || pixfmt == Format_YUV411P || pixfmt == Format_YUV422P
            || pixfmt == Format_YUV444P || pixfmt == Format_AYUV444
            || pixfmt == Format_IMC1 || pixfmt == Format_IMC2 || pixfmt == Format_IMC3 || pixfmt == Format_IMC4
            ;
    }

    bool CVideoFormat::isRGB(PixelFormat pixfmt)
    {
        return pixfmt == Format_RGB32 || pixfmt == Format_ARGB32
            || pixfmt == Format_RGB24 || pixfmt == Format_BGRA32
            || pixfmt == Format_ABGR32 || pixfmt == Format_RGBA32
            || pixfmt == Format_BGR565 || pixfmt == Format_RGB555 || pixfmt == Format_RGB565
            || pixfmt == Format_BGR24 || pixfmt == Format_BGR32 || pixfmt == Format_BGR555
            || pixfmt == Format_RGB48 || pixfmt == Format_RGB48LE || pixfmt == Format_RGB48BE
            || pixfmt == Format_BGR48 || pixfmt == Format_BGR48LE || pixfmt == Format_BGR48BE
            || pixfmt == Format_RGBA64 || pixfmt == Format_RGBA64LE || pixfmt == Format_RGBA64BE
            || pixfmt == Format_BGRA64 || pixfmt == Format_BGRA64LE || pixfmt == Format_BGRA64BE
            ;
    }

    bool CVideoFormat::hasAlpha(PixelFormat pixfmt)
    {
        return pixfmt == Format_ARGB32 || pixfmt == Format_BGRA32
            || pixfmt == Format_AYUV444;
    }

    void CVideoFormat::initBppInternal()
    {
        //TODO: call later when bpp need
        m_bpp = 0;
        m_bpp_pad = 0;
        //libavutil55: depth, step, offset
        m_bpc = DESC_VAL(m_pixdesc->comp[0].depth);
        const int log2_pixels = m_pixdesc->log2_chroma_w + m_pixdesc->log2_chroma_h;
        int steps[4];
        memset(steps, 0, sizeof(steps));
        for (int c = 0; c < m_pixdesc->nb_components; c++)
        {
            const AVComponentDescriptor *comp = &m_pixdesc->comp[c];
            int s = c == 1 || c == 2 ? 0 : log2_pixels; //?
            m_bpps[comp->plane] += DESC_VAL(comp->depth);
            steps[comp->plane] = DESC_VAL(comp->step) << s;
            m_channels[comp->plane] += 1;
            m_bpp += DESC_VAL(comp->depth) << s;
            if (DESC_VAL(comp->depth) != m_bpc)
            {
                m_bpc = 0;
            }
        }
        for (int i = 0; i < m_planes; ++i)
        {
            m_bpp_pad += steps[i];
        }
        if (!(m_pixdesc->flags & AV_PIX_FMT_FLAG_BITSTREAM))
        {
            m_bpp_pad *= 8;
        }
        m_bpp >>= log2_pixels;
        m_bpp_pad >>= log2_pixels;
    } 
    void CVideoFormat::initInternal(CVideoFormat::PixelFormat fmt)
    {
        m_pixfmt = fmt;
        m_pixfmt_ff = (AVPixelFormat)CVideoFormat::pixelFormatToFFmpeg(m_pixfmt);
        m_qpixfmt = CVideoFormat::imageFormatFromPixelFormat(m_pixfmt);
        initInternal();
    }
    void CVideoFormat::initInternal(QImage::Format fmt) {
        m_qpixfmt = fmt;
        m_pixfmt = CVideoFormat::pixelFormatFromImageFormat(fmt);
        m_pixfmt_ff = (AVPixelFormat)CVideoFormat::pixelFormatToFFmpeg(m_pixfmt);
        initInternal();
    }
    void CVideoFormat::initInternal(AVPixelFormat fffmt) {
        m_pixfmt_ff = fffmt;
        m_pixfmt = CVideoFormat::pixelFormatFromFFmpeg(m_pixfmt_ff);
        m_qpixfmt = CVideoFormat::imageFormatFromPixelFormat(m_pixfmt);
        initInternal();
    }

    void CVideoFormat::initInternal() {
        // TODO: what if other formats not supported by ffmpeg? give attributes in Kompass?
        if (m_pixfmt_ff == AV_PIX_FMT_NONE)
        {
            std::cout << "Invalid pixel format" << std::endl;
            return;
        }
        m_planes = qMax(av_pix_fmt_count_planes(m_pixfmt_ff), 0);
        m_bpps.reserve(m_planes);
        m_channels.reserve(m_planes);
        m_bpps.resize(m_planes);
        m_channels.resize(m_planes);
        m_pixdesc = const_cast<AVPixFmtDescriptor*>(av_pix_fmt_desc_get(m_pixfmt_ff));
        if (!m_pixdesc)
        {
            return;
        }
        initBppInternal();
    }
    std::string CVideoFormat::nameInternal() const
    {
        return av_get_pix_fmt_name(m_pixfmt_ff);
    }
    int CVideoFormat::flagsInternal() const
    {
        if (!m_pixdesc)
        {
            return 0;
        }
        return m_pixdesc->flags;
    }
    int CVideoFormat::bytesPerLineInternal(int width, int plane) const
    {
        return av_image_get_linesize(m_pixfmt_ff, width, plane);
    }
}
