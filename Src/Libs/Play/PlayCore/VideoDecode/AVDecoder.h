#pragma once

#include <QVariant>
#include <QObject>

#include "PlayCoreInclude.h"

class CAVDecoder
{
public:
    static const char* getProfileName(AVCodecID id, int profile);
    static const char* getProfileName(const AVCodecContext* ctx);

    CAVDecoder();
    virtual ~CAVDecoder();
     void setCodec(const AVCodec *codec);
    virtual std::string name() const;
    virtual std::string getDescription() const;
	virtual void restore();
    virtual bool open();
    virtual bool close();
    virtual bool getDecoderConfig() { return true; }
    virtual bool isValidTexture() { return true; }
    bool isOpen() const;
    virtual void flush();
    void setCodecContext(void* codecCtx);
    void* codecContext() const;
    bool isAvailable() const;
    int undecodedSize() const;

    void setCodecName(const std::string& name);
    std::string codecName() const;
    virtual void codecNameChanged() {}

    virtual bool openInternal() { return true; }
    virtual void closeInternal() {}
    virtual bool enableFrameRef() const { return true; }

protected:
    AVCodecContext *codec_ctx;
    const AVCodec *decoder;
    bool available;
    bool is_open;
    int undecoded_size;
    std::string codec_name;
    QVariantHash options;
    AVDictionary *dict;
};
