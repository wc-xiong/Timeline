#pragma once

#include <QString>
#include <QUuid>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

/** 多层硬解槽打开描述 */
struct SPreviewGpuLayerDesc
{
	QUuid clipId;
	QString path;
	qint64 seekMs = 0;
};

/**
 * @brief 硬解管线：单设备多槽 CMMDecoder（D3D11VA→RGBA）
 * 阶段5：最多 kMaxSlots 路并行，供预览多层合成。
 */
class CPreviewGpuPipeline
{
public:
	static constexpr int kMaxSlots = 4;

	static CPreviewGpuPipeline& Instance();

	bool InitDevice();
	void UninitDevice();
	bool IsDeviceReady() const { return m_bDeviceReady; }

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetContext();
	class BasicEffect* GetBasicEffect() const;

	/** 单路兼容（OpenFile / 软解回落前的主槽） */
	bool Open(const QString& path, bool bPreferHard);
	void Close();
	bool IsOpen() const;
	bool SeekMs(qint64 ms);
	void SetPlaying(bool playing);
	bool PumpFrame(qint64 clockMs, ID3D11ShaderResourceView** outSrv, int& outW, int& outH, qint64& outPtsMs);
	ID3D11Texture2D* GetRgbaTexture() const;
	int Width() const;
	int Height() const;
	QString Path() const;

	/** 阶段5：按可见层同步槽位（同 path+clipId 复用；多余关闭） */
	bool SyncLayers(const std::vector<SPreviewGpuLayerDesc>& layers, bool bPreferHard);
	void CloseLayers();
	int LayerCount() const;

	bool SeekLayer(const QUuid& clipId, qint64 ms);
	void SetLayersPlaying(bool playing);
	bool PumpLayer(const QUuid& clipId, qint64 clockMs,
		ID3D11ShaderResourceView** outSrv, ID3D11Texture2D** outTex,
		int& outW, int& outH, qint64& outPtsMs);
	ID3D11Texture2D* GetLayerTexture(const QUuid& clipId) const;

	/** 显式 FinishSeek（若 Pump 已在首帧自动 Finish 则为 no-op） */
	void FinishSeekLayer(const QUuid& clipId);
	void FinishSeekAll();
	bool IsAnySeeking() const;
	/**
	 * Seek 后有限次阻塞 Pump，直到各层 FinishSeek 或超时。
	 * 返回是否至少有一层出新帧。
	 */
	bool PrimeSeekFrames(int maxAttempts = 48, int sleepMs = 4);

private:
	CPreviewGpuPipeline();
	~CPreviewGpuPipeline();

	struct Impl;
	Impl* m_pImpl = nullptr;
	bool m_bDeviceReady = false;
	mutable std::mutex m_mtx;
};
