#pragma once
#include "GlobalModelGlobal.h"
#include <QString>
#include <QSettings>
#include <shared_mutex>
#include <QSize>

#include "Common/Common/IGlobalModelInterface.h"
#include "Common/Common/AdapterModeDefine.h"
#include "Common/Common/PlayCommand.h"

class GLOBALMODEL_EXPORT CPrjSettingData
{
public:
	static CPrjSettingData* GetInstance();

private:

	CPrjSettingData();
	CPrjSettingData(const CPrjSettingData&) = delete;
	CPrjSettingData& operator=(const CPrjSettingData&) = delete;
	~CPrjSettingData();

public:
	bool SaveToFile();

	void LoadSettings();

	void LoadDefault();

	// 素材信息
	/*int LoadIsResourceInfoShowMode() override;
	void SaveIsResourceInfoShow(int mode) override;*/

	// FPS
	int LoadFPSLevel();
	void SaveLoadFPSLevel(int level);

	// 解码显卡 DeviceId
	int LoadDecoderAdapter();
	void SaveDecoderAdapter(int deviceId);

	// 显卡模式
	int LoadDecoderAdapterMode();
	void SaveDecoderAdapterMode(int mode);

	// 物理显卡持久身份（Setup InstanceId / LocationPaths）
	QString LoadDecoderAdapterInstanceId();
	void SaveDecoderAdapterInstanceId(const QString& instanceId);
	QString LoadDecoderAdapterLocationPaths();
	void SaveDecoderAdapterLocationPaths(const QString& locationPaths);

	// 空格键功能
	int LoadSpaceKeyFunction();
	void SaveSpaceKeyFunction(int func);

	// 默认解码
	int LoadDefaultDecodingMode();
	void SaveDefaultDecodingMode(int mode);

	// 自动固定播放窗口
	bool LoadFixedPlayWnd();
	void SaveFixedPlayWnd(bool bFixedPlayWnd);

	// 默认音量
	double LoadDefaultVolume();
	void SaveDefaultVolume(double volume);

	// 图片默认时长
	int LoadDefaultPicDuration();
	void SaveDefaultPicDuration(int ms);

	// 小工具默认时长
	int LoadDefaultToolDuration();
	void SaveDefaultToolDuration(int ms);

	// PPT默认间隔
	int LoadDefaultTurnPageInterval();
	void SaveDefaultTurnPageInterval(int ms);

	// 流媒体默认时长
	int LoadDefaultStreamDuration();
	void SaveDefaultStreamDuration(int ms);

	// HDR
	std::pair<bool, int> LoadHDRSetting();
	void SaveHDRSetting(bool HDREnable, int mode);

	// 音频延迟
	int LoadAudioOutputDelay();
	void SaveAudioOutputDelay(int ms);

	// 淡入时间
	int LoadDefaultFadeInTime();
	void SaveDefaultFadeInTime(int time);

	// 淡出时间
	int LoadDefaultFadeOutTime();
	void SaveDefaultFadeOutTime(int time);

	QString LoadStageZoomRatio();
	void SaveStageZoomRatio(const QString &ratio);


	void SaveTimeLineTimeCodeStyle(bool bTimeCode);
	bool LoadTimeLineTimeCodeStyle();

	void SaveTimeLineFps(float fps);
	float LoadTimeLineFps();

	void SaveTimeLineDropFrame(bool dropFrame);
	bool LoadTimeLineDropFrame();

	/** @brief 读取路径相机动画片段默认时长（毫秒） */
	int LoadDefaultCameraAnimDuration();
	/** @brief 保存路径相机动画片段默认时长（毫秒） */
	void SaveDefaultCameraAnimDuration(int ms);

	// 局部解码（EPartialDecode：EPD_Close / EPD_Open；Invalid 按开启）
	int LoadPartialDecode();
	void SavePartialDecode(int mode);

	// 虚拟屏解码（EPartialDecode；局部解码关闭时强制 Close）
	int LoadVirtualScreenDecode();
	void SaveVirtualScreenDecode(int mode);

	/**
	 * @brief 读取三维视图「显示辅助对象图标」界面配置
	 * @return true=显示辅助对象图标（默认）
	 */
	bool LoadShowAuxiliaryObjects();
	/**
	 * @brief 保存三维视图「显示辅助对象图标」界面配置
	 * @param bShow true=显示；false=隐藏图标
	 * @note 仅更新内存字段，调用方需再调 SaveToFile 落盘
	 */
	void SaveShowAuxiliaryObjects(bool bShow);

	/**
	 * @brief 读取三维视图「显示所有对象名称」界面配置
	 * @return true=显示名称标签；默认 false
	 */
	bool LoadShowAllObjectNames();
	/**
	 * @brief 保存三维视图「显示所有对象名称」界面配置
	 * @param bShow true=显示名称标签；false=不显示
	 * @note 仅更新内存字段，调用方需再调 SaveToFile 落盘
	 */
	void SaveShowAllObjectNames(bool bShow);

private:
	int		m_nResourceInfoShowMode;
	int		m_nFPSLevel;
	int		m_nDefaultDecodingMode;
	int		m_nDefaultPicDuration;
	int		m_nDefaultToolDuration;
	int		m_nDefaultTurnPageInterval;
	int		m_nDefaultStreamDuration;
	int		m_nAudioOutDelay;
	int		m_nDefaultFadeInTime;
	int		m_nDefaultFadeOutTime;
	int		m_nDecoderAdapterId;
	int		m_nDecoderAdapterMode = EAM_Unvalid;
	QString	m_strDecoderAdapterInstanceId;
	QString	m_strDecoderAdapterLocationPaths;
	int		m_nSpaceKeyFunction = 0;
	bool	m_bFixedPlayWnd;
	double	m_fDefaultVol;
	QString m_strStageZoomRatio;

	std::pair<bool, int> m_HDRSetting;

	int m_nPPTEngineSelected = -1;	// PPT播放引擎选择（office or wps）
	int m_nPPTViewMode = -1;		// ppt获取画面模式

	bool m_bTimeLineTimeCodeStyle = true;	// 时间线样式
	float m_fTimeLineFps = 30.0f;			// 时间线帧率
	bool m_bTimeLineDropFrame = false;		// 时间线丢帧
	/** @brief 路径相机动画片段默认时长（毫秒），默认 10s */
	int m_nDefaultCameraAnimDuration = 1000 * 10;

	int m_nPartialDecode = EPD_Open;
	int m_nVirtualScreenDecode = EPD_Open;

	bool m_bShowAuxiliaryObjects = true;	// 三维视图辅助对象图标显示（界面配置）
	bool m_bShowAllObjectNames = false;		// 三维视图对象名称标签显示（界面配置）
};

// 工程设置
#define g_pProjectSetting CPrjSettingData::GetInstance()
