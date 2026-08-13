#pragma once

#include "Player.h"
#include "PlayerGlobal.h"
#include "wincodec.h"
#include <QString>
#include <QRunnable>

struct SGifDecoderCtx
{
	// GIF素材时长
	std::chrono::microseconds m_duration{ 0 };
	// GIF背景色
	D2D1_COLOR_F m_BackgroundColor;
	// GIF帧的绘制位置
	D2D1_RECT_F m_FramePosition;
};

struct SGifInfo;

class PLAYER_EXPORT CGifPlayer : public CBasePlayer
{
public:
	CGifPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx, const QString& strPath/*, std::shared_ptr<IShareProperty> pSharedProperty*/);
	virtual ~CGifPlayer();

	bool Prepare() override;
	bool GetTextView(const microseconds& curTime, SDrawTextureData& videoData) override;	// 获取纹理数据

	bool	Start(const microseconds& curTime) override;
	bool	Pause(const microseconds& curTime) override;
	bool	Stop() override;

	void InitThread();
	void UnInitThread();
	
protected:
	// 初始化D2D资源
	bool InitD2DResource();
	// 加载GIF图的信息
	void LoadGifInfo(SGifInfo* pInfo) const;
	// 根据播放进度推算应该解码GIF哪一帧
	long long GetExpectIndex(const std::chrono::microseconds& expectTime) const;
	// 从GIF头获取背景颜色信息
	HRESULT GetBackgroundColor(Microsoft::WRL::ComPtr<IWICMetadataQueryReader> pMetadataQueryReader, DWORD &dwBGColor) const;
	// GIF帧解码循环
	void DecodeGifInLoop();
	// 加载Raw帧
	HRESULT LoadRawFrame();
	// 清理D2D画布
	HRESULT ClearOnD2DTarget();
	// 将GIF中的Raw帧合成到D2D画布上
	HRESULT RenderOnD2DTarget();
	// 纹理更新
	void UpdateImageInfo();

	std::chrono::milliseconds WorkImp();

protected:
	QString m_strFilePath;
	SGifDecoderCtx m_gifDecoderCtx;
	bool m_bPrepared = false;
	bool m_bRun = false;
	
	// 2D绘图工厂
	ComPtr<ID2D1Factory>			m_p2dfactory = nullptr;
	// WIC工厂
	ComPtr<IWICImagingFactory> m_pWICfactory = nullptr;
	// WIC解码器
	ComPtr<IWICBitmapDecoder> m_pWICdecoder = nullptr;
	// GIF信息
	std::shared_ptr<SGifInfo> m_pGifInfo = nullptr;
	// GIF帧合成画布
	ComPtr<ID2D1RenderTarget> m_pComposeRT = nullptr;
	// GIF帧合成画布的D3D纹理引用
	ComPtr<ID3D11Texture2D> m_pComposeTexture = nullptr;

	// GIF读取出来的Raw帧
	ComPtr<ID2D1Bitmap> m_pRawFrame = nullptr;
	
	// 解码第几帧的控制参数
	std::mutex m_NextFrameIndexMutex;
	long long m_NextFrameIndex = -1;
	long long m_NowFrameIndex = -1;

	std::thread* m_pUpdateTrd = nullptr;	//纹理更新线程
};

class AutoSectionLocker final
{
public:
	explicit AutoSectionLocker(LPCRITICAL_SECTION section) noexcept :mSection(section) { EnterCriticalSection(mSection); }
	~AutoSectionLocker() { LeaveCriticalSection(mSection); }

	AutoSectionLocker(const AutoSectionLocker&) = delete;
	AutoSectionLocker& operator=(const AutoSectionLocker&) = delete;

private:
	LPCRITICAL_SECTION mSection;
};