#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
//#include <libavcodec/dxva2.h> 
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/cpu.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/common.h>
#include <libavutil/time.h>
}

#include <libavcodec/d3d11va.h>
#include <libavutil/hwcontext_d3d11va.h>

#define AV_ENSURE_OK(FUNC, ...) AV_RUN_CHECK(FUNC, return, __VA_ARGS__)

#define AV_RUN_CHECK(FUNC, RETURN, ...) do { \
    int ret = FUNC; \
    if (ret < 0) { \
        char str[AV_ERROR_MAX_STRING_SIZE]; \
        memset(str, 0, sizeof(str)); \
        av_strerror(ret, str, sizeof(str)); \
        av_log(NULL, AV_LOG_WARNING, "Error " #FUNC " @%d " __FILE__ ": (%#x) %s\n", __LINE__, ret, str); \
        RETURN __VA_ARGS__; \
     } } while(0)

#define CLASS_DISABLE_COPY(ClassName) private:\
ClassName(const ClassName &);\
ClassName & operator=(const ClassName &);