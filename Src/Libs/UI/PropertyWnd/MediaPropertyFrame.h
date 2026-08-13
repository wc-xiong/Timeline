#pragma once
#include "PropertyFrame.h"
#include "Common/Maths/Matrix3x2.h"
#include <QUuid>
#include <chrono>
#include <optional>
#include <set>
#include <vector>

class CComboBoxWidget;
class CLineEditWidget;
class CGroupWidgetBox;
class CDoubleSpinBoxWidget;
class CTimeEditWidget;
class CSliderWidget;
class CCheckBoxEx;
class CLabelSpinbox;
class CPropertyResetBtn;
class CPropertyKeyFrameBar;
class CSvgImageButton;
class QFrame;
class QLabel;
class QWidget;

/** 特效属性面板分组/行 ID（公共定义，见 Common/MediaPropertyKeyframeIds.h） */
#include "Common/MediaPropertyKeyframeIds.h"

/**
 * @brief 媒体属性面板
 *
 * 单选素材时展示基础属性、音量/位置/缩放/旋转、裁剪/羽化/抠像、色彩效果及音频路由。
 * 可关键帧属性通过 CMediaKeyframeHelper 与时间线同步；游标不在素材内时禁用滑条/SpinBox 编辑。
 */
class CMediaPropertyFrame : public CPropertyFrame
{
	Q_OBJECT
public:
	CMediaPropertyFrame(QWidget* pParent = nullptr);
	virtual ~CMediaPropertyFrame();
	/**
	 * @brief 选中媒体变化或外部刷新时，重载全部控件显示与 enable 状态
	 * @details 读取时间线选中集，同步名称/时间/特效/音频等，并刷新关键帧 Toggle 与插值显示
	 */
	void UpdateProperty() override;
	void UpdateTranslateText();
	float GetPropertyKeyframeValue(EMediaPropertyId propId) const;
	void SetPropertyDisplayValue(EMediaPropertyId propId, float value);
	const std::set<QUuid>& GetSelectedMediaIds() const { return m_setMediaIDs; }
	/**
	 * @brief 刷新各行/分组关键帧 Toggle 按钮状态
	 * @param playheadMs 全局游标时间（毫秒）；省略时使用当前播放头位置
	 */
	void RefreshPropertyKeyframeToggleStates(std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	/**
	 * @brief 按当前游标位置同步插值后的属性显示值
	 * @param playheadMs 全局游标时间（毫秒）；省略时使用当前播放头位置
	 */
	void SyncKeyframeEvaluatedValuesAtPlayhead(std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);

protected:
	void InitWidget();
	void CreateConnects();
	void ClearEdit();
	void EnableEdit();
	/**
	 * @brief 翻页滚动类型切换时更新滚动速度/首尾间隔等子控件可见性
	 * @param bSelectPreIndex 是否选中「按页/pre」类滚动模式
	 */
	void UpdateTurnPageScrollCtrl(bool bSelectPreIndex);
	/**
	 * @brief 按素材资源类型显示/隐藏对应属性分组
	 * @param resType 资源类型枚举（视频/图片/文字/音频等）
	 */
	void UpdateGroupVisibility(int resType);
	/** @brief 根据各分组可见性刷新组间分隔线显示状态 */
	void UpdateGroupSeparatorVisibility();
	/**
	 * @brief 将指定属性分组重置为默认值并刷新 UI
	 * @param groupId 目标分组 ID
	 */
	void ResetPropertyGroup(EMediaPropertyGroupId groupId);
	/**
	 * @brief 连接分组标题上的重置按钮与分组关键帧栏信号
	 * @param groupId 分组 ID
	 * @param pResetBtn 分组重置按钮
	 * @param pKeyFrameBar 分组级关键帧栏（整组 Toggle/导航）
	 */
	void ConnectGroupHeaderActions(EMediaPropertyGroupId groupId,
		CPropertyResetBtn* pResetBtn,
		CPropertyKeyFrameBar* pKeyFrameBar);
	/**
	 * @brief 连接单行属性关键帧栏与统一槽函数
	 * @param pBar 属性行关键帧栏指针
	 * @param propId 对应的属性行 ID
	 */
	void ConnectPropertyKeyFrameBar(CPropertyKeyFrameBar* pBar, EMediaPropertyId propId);
	/** @brief 根据当前选中素材刷新音频路由相关控件（类型/声道映射） */
	void UpdateAudioPropertyUI();
	/** @brief 声道映射变更后通知网络层推送到播放端 */
	void NotifyMediaAudioChannelMapChanged(const QUuid& mediaId);
	void NotifyMediaPropertyKeyframesChanged(const QUuid& mediaId);
	/** @brief 音量强度变更后通知网络层推送到播放端 */
	void NotifyMediaVolumeChanged(const QUuid& mediaId);
	/** @brief 枚举系统音频输出设备并刷新下拉框选项 */
	void RefreshAudioOutputDeviceCombo();
	/**
	 * @brief 统一控制关键帧栏与特效编辑控件的 enable 状态
	 * @param bEnabled 图层未锁时为 true
	 * @param playheadMs 传入时用于判断游标是否在素材内；仅当 bEnabled 且游标在素材内时允许编辑滑条/SpinBox
	 */
	void SetPropertyActionBarsEnabled(bool bEnabled, std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	/**
	 * @brief 暂时隐藏位置 / 缩放 / 旋转 / 裁剪组的关键帧操作栏
	 * @details 对应组的滑条与编辑框仍可编辑，但不打关键帧；在
	 *          SetPropertyActionBarsEnabled / RefreshPropertyKeyframeToggleStates 之后调用，
	 *          避免刷新关键帧栏时把已隐藏栏重新显示
	 */
	void DisableTemporarilyUnsupportedPropertyGroups();
	/**
	 * @brief 在播放头位于素材内时，将编辑值写入模型并打关键帧
	 * @param propId 属性行 ID
	 * @param value 用户编辑后的属性值
	 */
	void ApplyPropertyEditAtPlayhead(EMediaPropertyId propId, float value);
	/**
	 * @brief 根据资源类型与声道数生成音频类型展示文本
	 * @param nResType 资源类型
	 * @param nChannels 输入声道数
	 * @return 本地化后的类型描述字符串
	 */
	QString BuildAudioTypeText(int nResType, int nChannels) const;
	/**
	 * @brief 按素材声道数刷新声道映射下拉框选项与可见行数
	 * @param nInputChannels 素材输入声道数
	 */
	void UpdateChannelMapComboItems(int nInputChannels);

protected slots:
	void slotDoubleSpinBoxEdit(float);
	void slotLineEdit(const QString &text);
	void slotTimeEdit();
	void slotTimeEditFinish();
	void slotEffectPropertyValueChange(double fValue);
	void slotEffectPropertyValueChangeMouseRelease(double fValue);
	void slotFillComboSelectChange(int nIndex);
	void slotComboTurnPageScrollTypeChange(int nIndex);
	void slotTurnPageIntervalChange(int nValue);
	void slotEndToEndIntervalChange(int nValue);
	void slotEteBtnClicked();
	void slotUpdateScrollProperty(bool b);
	void slotAudioChannelMapCheckChanged();
	void slotAudioChannelChanged(int nIndex);
	void slotCropSliderChange(double fValue);
	void slotFeatherSliderChange(double fValue);
	void slotKeyingSliderChange(double fValue);
	void slotPropertyKeyFrameStub(EMediaPropertyId propId, EKeyFrameOp op);
	void slotGroupKeyFrameStub(EMediaPropertyGroupId groupId, EKeyFrameOp op);
	/** @brief 缩放宽高等比例锁定按钮切换 */
	void slotScaleAspectLockClicked();

private:
	/** @brief 单选素材时的属性编辑主容器 Frame */
	QFrame* m_pOneMediaFrame = nullptr;
	/** @brief 素材名称单行编辑控件 */
	CLineEditWidget* m_pNameLineEditWidget = nullptr;
	/** @brief 时间线起始时间编辑控件 */
	CTimeEditWidget* m_pPlayStart = nullptr;
	/** @brief 时间线持续时长编辑控件 */
	CTimeEditWidget* m_pPlayDuration = nullptr;
	/** @brief 视频类素材专有属性容器（入点/出点等） */
	QFrame* m_pFrameVideoProperty = nullptr;
	/** @brief 素材入点（Clip Begin）时间编辑控件 */
	CTimeEditWidget* m_pClipBegin = nullptr;
	/** @brief 素材出点（Clip End）时间编辑控件 */
	CTimeEditWidget* m_pClipEnd = nullptr;
	/** @brief 样式属性容器（填充方式等） */
	QFrame* m_pFrameStyleProperty = nullptr;
	/** @brief 画面填充方式下拉框（拉伸/适应/裁剪等） */
	CComboBoxWidget* m_pComboFillType = nullptr;
	/** @brief 文字滚动类型选择区域容器 */
	QFrame* m_pFrameScrollType = nullptr;
	/** @brief 文字翻页/滚动类型下拉框 */
	CComboBoxWidget* m_pComboTextScrollType = nullptr;
	/** @brief 滚动速度编辑区域容器 */
	QFrame* m_pFrameScrollSpeed = nullptr;
	/** @brief 文字滚动速度 SpinBox */
	CLabelSpinbox* m_pEditTextScrollSpeed = nullptr;
	/** @brief 滚动相关属性子控件总容器 */
	QFrame* m_pScrollPtyContainer = nullptr;
	/** @brief 「首尾相连」滚动模式复选框 */
	CCheckBoxEx* m_pEteBtn = nullptr;
	/** @brief 首尾衔接间隔 SpinBox */
	CLabelSpinbox* m_pEndToEndInterval = nullptr;
	/** @brief 基础属性分组（名称/时间/样式/滚动） */
	CGroupWidgetBox* m_pBasePropertyGroup = nullptr;
	// --- 音频属性 ---
	/** @brief 音频路由属性分组容器 */
	CGroupWidgetBox* m_pAudioPropertyGroup = nullptr;
	/** @brief 音频类型展示行外层包装 Widget */
	QWidget* m_pWrapAudioType = nullptr;
	/** @brief 音频类型名称标签（「类型」） */
	QLabel* m_pLabelAudioTypeName = nullptr;
	/** @brief 音频类型值标签（只读，如「立体声」） */
	QLabel* m_pLabelAudioTypeValue = nullptr;
	/** @brief 声道映射行外层包装 Widget */
	QWidget* m_pWrapAudioChannelMap = nullptr;
	/** @brief 声道映射名称标签（「声道」） */
	QLabel* m_pLabelAudioChannelMapName = nullptr;
	/** @brief 声道映射启用复选框 */
	CCheckBoxEx* m_pChkAudioChannelMap = nullptr;
	/** @brief 声道映射下拉框列表（最多 CHANNEL_SYSTEM_MAX 路，按素材声道数显示） */
	std::vector<CComboBoxWidget*> m_vecChannelCombobox;
	/** @brief 当前素材声道系统类型（通道数，取自 SChannelMap::type） */
	unsigned int m_nAudioChannelType = 0;
	// --- 音量 ---
	/** @brief 音量属性分组容器 */
	CGroupWidgetBox* m_pVolumePropertyGroup = nullptr;
	/** @brief 音量强度滑条（0~100% 或等效范围） */
	CSliderWidget* m_pVolume = nullptr;
	/** @brief 音量分组重置按钮 */
	CPropertyResetBtn* m_pVolumeResetBtn = nullptr;
	/** @brief 音量分组级关键帧栏（整组 Toggle/导航） */
	CPropertyKeyFrameBar* m_pVolumeGroupKeyFrameBar = nullptr;
	/** @brief 音量强度行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfVolume = nullptr;
	// --- 位置 ---
	/** @brief 位置属性分组容器 */
	CGroupWidgetBox* m_pPositionPropertyGroup = nullptr;
	/** @brief X 轴位置 DoubleSpinBox */
	CDoubleSpinBoxWidget* m_pX = nullptr;
	/** @brief Y 轴位置 DoubleSpinBox */
	CDoubleSpinBoxWidget* m_pY = nullptr;
	/** @brief Z 轴位置 DoubleSpinBox（深度/层级，UI 本地缓存 m_fPosZ） */
	CDoubleSpinBoxWidget* m_pZ = nullptr;
	/** @brief 位置分组重置按钮 */
	CPropertyResetBtn* m_pPositionResetBtn = nullptr;
	/** @brief 位置分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pPositionGroupKeyFrameBar = nullptr;
	/** @brief X 位置行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfPosX = nullptr;
	/** @brief Y 位置行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfPosY = nullptr;
	/** @brief Z 位置行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfPosZ = nullptr;
	// --- 缩放 ---
	/** @brief 缩放属性分组容器 */
	CGroupWidgetBox* m_pScalePropertyGroup = nullptr;
	/** @brief 宽高等比例锁定按钮（写入 KeepAspectRatio） */
	CSvgImageButton* m_pScaleAspectLockBtn = nullptr;
	/** @brief 宽度缩放 DoubleSpinBox */
	CDoubleSpinBoxWidget* m_pWidth = nullptr;
	/** @brief 高度缩放 DoubleSpinBox */
	CDoubleSpinBoxWidget* m_pHeight = nullptr;
	/** @brief 缩放分组重置按钮 */
	CPropertyResetBtn* m_pScaleResetBtn = nullptr;
	/** @brief 缩放分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pScaleGroupKeyFrameBar = nullptr;
	/** @brief 宽度缩放行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfScaleWidth = nullptr;
	/** @brief 高度缩放行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfScaleHeight = nullptr;
	// --- 旋转 ---
	/** @brief 旋转属性分组容器 */
	CGroupWidgetBox* m_pRotationPropertyGroup = nullptr;
	/** @brief 绕 X 轴旋转 DoubleSpinBox（UI 本地缓存 m_fRotX） */
	CDoubleSpinBoxWidget* m_pRotX = nullptr;
	/** @brief 绕 Y 轴旋转 DoubleSpinBox（UI 本地缓存 m_fRotY） */
	CDoubleSpinBoxWidget* m_pRotY = nullptr;
	/** @brief 绕 Z 轴旋转 DoubleSpinBox */
	CDoubleSpinBoxWidget* m_pRotZ = nullptr;
	/** @brief 旋转分组重置按钮 */
	CPropertyResetBtn* m_pRotationResetBtn = nullptr;
	/** @brief 旋转分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pRotationGroupKeyFrameBar = nullptr;
	/** @brief X 旋转行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfRotX = nullptr;
	/** @brief Y 旋转行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfRotY = nullptr;
	/** @brief Z 旋转行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfRotZ = nullptr;
	// --- 裁剪（UI 缓存 + CFxProperty::Cut）---
	/** @brief 裁剪属性分组容器 */
	CGroupWidgetBox* m_pCropPropertyGroup = nullptr;
	/** @brief 裁剪填充行外层包装 Widget */
	QWidget* m_pWrapCropFill = nullptr;
	/** @brief 「裁剪填充」标签 */
	QLabel* m_pLabelCropFill = nullptr;
	/** @brief 裁剪是否填充复选框（对应 m_bCropFill / CropFill 关键帧） */
	CCheckBoxEx* m_pChkCropFill = nullptr;
	/** @brief 裁剪左边距滑条（对应 m_fCropLeft） */
	CSliderWidget* m_pCropLeft = nullptr;
	/** @brief 裁剪右边距滑条（对应 m_fCropRight） */
	CSliderWidget* m_pCropRight = nullptr;
	/** @brief 裁剪上边距滑条（对应 m_fCropTop） */
	CSliderWidget* m_pCropTop = nullptr;
	/** @brief 裁剪下边距滑条（对应 m_fCropBottom） */
	CSliderWidget* m_pCropBottom = nullptr;
	/** @brief 裁剪分组重置按钮 */
	CPropertyResetBtn* m_pCropResetBtn = nullptr;
	/** @brief 裁剪分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pCropGroupKeyFrameBar = nullptr;
	/** @brief 裁剪填充行关键帧栏 */
	/** @brief 裁剪左边距行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfCropLeft = nullptr;
	/** @brief 裁剪右边距行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfCropRight = nullptr;
	/** @brief 裁剪上边距行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfCropTop = nullptr;
	/** @brief 裁剪下边距行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfCropBottom = nullptr;
	// --- 羽化 ---
	/** @brief 羽化属性分组容器 */
	CGroupWidgetBox* m_pFeatherPropertyGroup = nullptr;
	/** @brief 羽化半径滑条（对应 m_fFeatherRadius） */
	CSliderWidget* m_pFeatherRadius = nullptr;
	/** @brief 羽化分组重置按钮 */
	CPropertyResetBtn* m_pFeatherResetBtn = nullptr;
	/** @brief 羽化分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pFeatherGroupKeyFrameBar = nullptr;
	/** @brief 羽化半径行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfFeatherRadius = nullptr;
	// --- 抠像 ---
	/** @brief 抠像（色度键）属性分组容器 */
	CGroupWidgetBox* m_pKeyingPropertyGroup = nullptr;
	/** @brief 抠像目标色 R 分量滑条（对应 m_fKeyingRed） */
	CSliderWidget* m_pKeyingRed = nullptr;
	/** @brief 抠像目标色 G 分量滑条（对应 m_fKeyingGreen） */
	CSliderWidget* m_pKeyingGreen = nullptr;
	/** @brief 抠像目标色 B 分量滑条（对应 m_fKeyingBlue） */
	CSliderWidget* m_pKeyingBlue = nullptr;
	/** @brief 抠像容差滑条（对应 m_fKeyingTolerance） */
	CSliderWidget* m_pKeyingTolerance = nullptr;
	/** @brief 抠像边缘柔化程度滑条（对应 m_fKeyingDegree） */
	CSliderWidget* m_pKeyingDegree = nullptr;
	/** @brief 抠像分组重置按钮 */
	CPropertyResetBtn* m_pKeyingResetBtn = nullptr;
	/** @brief 抠像分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pKeyingGroupKeyFrameBar = nullptr;
	/** @brief 抠像 R 行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfKeyingRed = nullptr;
	/** @brief 抠像 G 行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfKeyingGreen = nullptr;
	/** @brief 抠像 B 行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfKeyingBlue = nullptr;
	/** @brief 抠像容差行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfKeyingTolerance = nullptr;
	/** @brief 抠像柔化程度行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfKeyingDegree = nullptr;
	/** @brief 撤销栈用：编辑前记录的起始时间（毫秒） */
	std::chrono::milliseconds m_msOldStartTime;
	/** @brief 撤销栈用：编辑前记录的持续时长（毫秒） */
	std::chrono::milliseconds m_msOldDuration;
	/** @brief 多选素材时的公共时间范围编辑容器 */
	QFrame* m_pMutilMediaFrame = nullptr;
	/** @brief 多选时公共起始时间编辑控件 */
	CTimeEditWidget* m_pMutilMediaPlayStart = nullptr;
	/** @brief 多选时公共持续时长编辑控件 */
	CTimeEditWidget* m_pMutilMediaPlayDuration = nullptr;
	// --- 色彩效果（CEffectProperty）---
	/** @brief 色彩效果属性分组容器 */
	CGroupWidgetBox* m_pEffectPropertyGroup = nullptr;
	/** @brief 亮度滑条 */
	CSliderWidget* m_pLuminance = nullptr;
	/** @brief 红色偏移滑条 */
	CSliderWidget* m_pRed = nullptr;
	/** @brief 绿色偏移滑条 */
	CSliderWidget* m_pGreen = nullptr;
	/** @brief 蓝色偏移滑条 */
	CSliderWidget* m_pBlue = nullptr;
	/** @brief 透明度滑条 */
	CSliderWidget* m_pAlpha = nullptr;
	/** @brief 对比度滑条 */
	CSliderWidget* m_pContract = nullptr;
	/** @brief 色相滑条 */
	CSliderWidget* m_pHue = nullptr;
	/** @brief 饱和度滑条 */
	CSliderWidget* m_pSaturability = nullptr;
	/** @brief 红色偏移行外层包装 Widget */
	QWidget* m_pWrapEffectRed = nullptr;
	/** @brief 绿色偏移行外层包装 Widget */
	QWidget* m_pWrapEffectGreen = nullptr;
	/** @brief 蓝色偏移行外层包装 Widget */
	QWidget* m_pWrapEffectBlue = nullptr;
	/** @brief 色彩效果分组重置按钮 */
	CPropertyResetBtn* m_pEffectResetBtn = nullptr;
	/** @brief 色彩效果分组级关键帧栏 */
	CPropertyKeyFrameBar* m_pEffectGroupKeyFrameBar = nullptr;
	/** @brief 红色偏移行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectRed = nullptr;
	/** @brief 绿色偏移行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectGreen = nullptr;
	/** @brief 蓝色偏移行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectBlue = nullptr;
	/** @brief 透明度行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectAlpha = nullptr;
	/** @brief 亮度行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectLuminance = nullptr;
	/** @brief 对比度行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectContrast = nullptr;
	/** @brief 色相行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectHue = nullptr;
	/** @brief 饱和度行关键帧栏 */
	CPropertyKeyFrameBar* m_pKfEffectSaturation = nullptr;
	/** @brief 属性分组间分隔线数量（与 EMediaPropertyGroupId 分组数对应） */
	enum { EGroupSeparatorCount = 9 };
	/** @brief 各属性分组之间的水平分隔线控件数组 */
	QWidget* m_pGroupSeparators[EGroupSeparatorCount] = {};
	/** @brief 色彩效果 RGB 子区域与亮度/对比度区域之间的分隔线 */
	QWidget* m_pWrapEffectRgbSplitLine = nullptr;
	/** @brief 基础属性组内「样式/滚动」区上方的分隔线 */
	QWidget* m_pStyleSplitLine = nullptr;
	// UI-only 本地缓存（与 SetPropertyDisplayValue / GetPropertyKeyframeValue 同步，默认值对齐 XEffectCommon）
	/** @brief Z 轴位置 UI 本地缓存（模型无独立 Z 时使用） */
	float m_fPosZ = 0.f;
	/** @brief 绕 X 轴旋转 UI 本地缓存 */
	float m_fRotX = 0.f;
	/** @brief 绕 Y 轴旋转 UI 本地缓存 */
	float m_fRotY = 0.f;
	/** @brief 裁剪填充开关 UI 本地缓存 */
	bool m_bCropFill = false;
	/** @brief 裁剪左边距 UI 本地缓存（归一化 0~1） */
	float m_fCropLeft = 0.f;
	/** @brief 裁剪右边距 UI 本地缓存（归一化 0~1） */
	float m_fCropRight = 0.f;
	/** @brief 裁剪上边距 UI 本地缓存（归一化 0~1） */
	float m_fCropTop = 0.f;
	/** @brief 裁剪下边距 UI 本地缓存（归一化 0~1） */
	float m_fCropBottom = 0.f;
	/** @brief 羽化半径 UI 本地缓存，默认 0.3 对齐 SFeatherFactor */
	float m_fFeatherRadius = 0.3f;
	/** @brief 抠像目标色 R UI 本地缓存 */
	float m_fKeyingRed = 0.f;
	/** @brief 抠像目标色 G UI 本地缓存，默认 1 对齐绿幕 */
	float m_fKeyingGreen = 1.f;
	/** @brief 抠像目标色 B UI 本地缓存 */
	float m_fKeyingBlue = 0.f;
	/** @brief 抠像容差 UI 本地缓存 */
	float m_fKeyingTolerance = 0.5f;
	/** @brief 抠像边缘柔化程度 UI 本地缓存 */
	float m_fKeyingDegree = 0.1f;

private:
	/**
	 * @brief 按属性行 ID 查找对应的关键帧栏控件
	 * @param propId 属性行 ID
	 * @return 匹配的 CPropertyKeyFrameBar 指针；未找到时返回 nullptr
	 */
	CPropertyKeyFrameBar* FindPropertyKeyFrameBar(EMediaPropertyId propId) const;
	std::set<QUuid> m_setMediaIDs;
};
