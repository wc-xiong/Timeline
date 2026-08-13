#pragma once
#include "MediaModelGlobal.h"
#include "Common/IMediaPropertyInterface.h"
#include "Common/MediaCommon.h"
#include "Common/ChannelMap.h"
#include <mutex>

class MEDIAMODEL_EXPORT CShareProperty 
	: public IShareProperty
	, public std::enable_shared_from_this<CShareProperty>
{
public:
	CShareProperty();
	virtual ~CShareProperty();

	CShareProperty(const CShareProperty& data);
	CShareProperty(std::shared_ptr<IShareProperty> pdata);
	CShareProperty& operator=(const CShareProperty& data);

public:
	void SetVolume(float f) override;
	float GetVolume() override;

	virtual void SetMediaAudioSpeaker(const QString & name) override;
	virtual QString GetMediaAudioSpeaker() override;


	void SetMuted(bool bMuted) override;
	bool GetMuted() override;

	// 开始播放时间
	__int64 GetStartPlayTime() const override;
	void SetStartPlayTime(__int64 time) override;

	// 媒体翻页，滚动数据
	void SetControllType(int nType) override;
	int GetControllType() override;

	int GetPageWidth()override;
	void SetPageWidth(int w) override;

	int GetPageHeight()override;
	void SetPageHeight(int h)override;

	int GetPageCount()override;
	void SetPageCount(int count)override;

	void SetAutoTurnInterval(__int64 nMs) override;
	__int64 GetAutoTurnInterval() override;

	std::shared_ptr<SMediaScrollParam> GetScrollParam() override;
	void SetScrollParam(std::shared_ptr<SMediaScrollParam> pParam) override;
	void SetScrollParamData(SMediaScrollParam scrollParam) override;

	std::shared_ptr<SMediaPageTurnFxInfo> GetPageTurnFxInfo() override;					// 翻页特效
	void SetPageTurnFxInfo(std::shared_ptr<SMediaPageTurnFxInfo> pFxInfo) override;
	void SetPageTurnFxInfoData(SMediaPageTurnFxInfo TurnFxInfo) override;

	void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	void LoadDataFromRapidJson(const rapidjson::Value &root) override;

	void CopyProperty(std::shared_ptr<IShareProperty> pNode) override;

	void ResetPlaySetting() override;
	void ResetVoiceSetting() override;

	void SetStitchGrpResBaseSize(const QSize &sz) override {
		m_szStitchGrpResBase = sz;
	}
	QSize GetStitchGrpResBaseSize() override {
		return m_szStitchGrpResBase;
	}

	virtual void SetUsedChannelMap(bool bUse) override;
	virtual bool GetUsedChannelMap() override;

	virtual void SetAudioChannelMap(const SChannelMap& map) override;
	virtual SChannelMap GetAudioChannelMap() override;

private:
	__int64 m_nStartPlayTime;			// 开始播放时间 

	float m_fVolume;
	bool m_bMuted;						// 是否静音

	// 媒体控制模式
	std::atomic<int> m_nMediaControllType = 0;
	 
	int m_nPageWidth = 1920;
	int m_nPageHeight = 1080;

	std::atomic<__int64> m_nAutoTurnInterval = 10000;

	int m_nPageCount = 0;

	std::shared_ptr <SMediaScrollParam> m_pScrollParam;

	std::shared_ptr <SMediaPageTurnFxInfo> m_pPageTurnFxInfo;			// 翻页特效


	QSize m_szStitchGrpResBase;	// 拼接合集素材基准分辨率

	std::mutex m_Mutex;

	QString		m_strSpeakerName;
	bool		m_bUseChannelMap = false;		// 声道映射
	SChannelMap m_channelMap;			// 声道映射参数
};
