#pragma once

#include <QPixmap>
#include <QUuid>
#include <vector>
#include <functional>
#include <QTime>
#include <QPoint>
#include <QSize>
#include <QRectF>
#include <rapidjson/document.h>
#include <QPixmap>
#include "IFxInterface.h"
#include <set>
#include <memory>

struct SDrawTextureData;
struct SPlayClipInfo;
struct SExcelScrollParam;

//////////////////////////////////////////////////////////////////////////
class IMediaBaseProperty
{
public:
	virtual ~IMediaBaseProperty() {};

	virtual bool IsDirty() = 0;
	virtual void SetDirty(bool b) = 0;

	virtual void SetResId(const QUuid& id) =0;
	virtual QUuid GetResId() = 0;

	virtual void SetMediaId(const QUuid& id) = 0;
	virtual QUuid GetMediaId() = 0;

	virtual void SetResType(int iType) = 0;
	virtual int GetResType() = 0;

	virtual void SetName(const QString& name) = 0;
	virtual QString& GetName() = 0;

	virtual bool IsHWDEC() = 0;
	virtual std::vector<QString> GetInfo() = 0;
	virtual QPixmap GetThumbnail() = 0;

	// 解码偏好（EDecoding：自动 / 软解 / 硬解），上时间线时从素材快照
	virtual void SetDecodingMode(int mode) = 0;
	virtual int GetDecodingMode() = 0;

	virtual void SetOriginalSize(QSize& sz) = 0;
	virtual QSize GetOriginalSize() = 0;

	virtual void SetRotatable(bool bRotatable) = 0;
	virtual bool GetRotatable() = 0;

	virtual void SetLocked(bool bLocked) = 0;
	virtual bool GetLocked() = 0;

	virtual void SetFixed(bool bLocked) = 0;
	virtual bool GetFixed() = 0;

	virtual void SetHided(bool bHided) = 0;
	virtual bool GetHided() = 0;

	virtual void SetPos(QPoint ptPos) = 0;
	virtual QPointF GetPos() = 0;

	virtual void SetSize(QSize szSize) = 0;
	virtual QSizeF GetSize() = 0;

	virtual void SetRotate(float nRotate) = 0;
	virtual float GetRotate() = 0;

	virtual void SetKeepAspectRatio(bool bKeepAspectRatio) = 0;
	virtual bool GetKeepAspectRatio() = 0;

	virtual void SetSizeKeepAspectRatio(QSizeF szSize) = 0;
	virtual QSizeF GetSizeKeepAspectRatio() = 0;

	virtual void SetFillType(int type) = 0;
	virtual int GetFillType() = 0;

	virtual void SetBlendType(int type) = 0;
	virtual int GetBlendType() = 0;

	virtual void SetWidth(int iValue) = 0;
	virtual int GetWidth() const = 0;

	virtual void SetHeight(int iValue) = 0;
	virtual int GetHeight() const = 0;

	virtual void SetHorizontal(bool b) = 0;
	virtual bool GetHorizontal() = 0;

	virtual void SetVertical(bool b) = 0;
	virtual bool GetVertical() = 0;

	virtual QRectF GetWrapRect() = 0;
	virtual void GetPoints(std::vector<QPointF>&vecPoints) = 0;
	virtual void GetPointsWithoutFilp(std::vector<QPointF>&vecPoints) = 0;

	virtual QPointF GetFixedRectCenterWithoutRotate(float fradio) = 0;
	virtual QPointF GetLockRectCenterWithoutRotate(float fradio) = 0;
	virtual QPointF GetFxRectCenterWithoutRotate(float fradio) = 0;
	virtual QPointF GetRotateRectCenter(float fradio) = 0;

	virtual QPointF GetCenter() = 0;

	virtual void Move(float xoffset, float yoffset) = 0;
	virtual void Scale(float xradio, float yradio, float centerx, float centery) = 0;
	virtual void Scale(float xradio, float yradio) = 0;
	virtual void Rotate(float angleoffest, float centerx, float centery) = 0;
	virtual void Rotate(float angleoffset) = 0;

	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
	virtual void LoadDataFromRapidJson(const rapidjson::Value& root) = 0;

	virtual QRectF GetSizeRect() = 0;

	virtual void Replace() = 0;

	virtual void CopyProperty(std::shared_ptr<IMediaBaseProperty> pNode) = 0;
};

class IFxProperty 
{
public:
	virtual ~IFxProperty() {};

	virtual void ClearAllFxFacyor() = 0;
	virtual void DeleteFxFactor(int type) = 0;
	virtual void AddFxFaxtor(std::shared_ptr<IFxFactor> pFxFactor, int nIndex = -1) = 0;
	virtual std::shared_ptr<IFxFactor> GetFxFactor(int type) = 0;
	virtual std::shared_ptr<IFxFactor> GetFxFactorWithIndex(int type, int& nIndex) = 0;

	virtual std::vector<std::shared_ptr<IFxFactor>> GetAllFxFactor() = 0;

	virtual bool hasFactor() = 0;

	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
	virtual void LoadDataFromRapidJson(const rapidjson::Value& root) = 0;

	virtual void CopyProperty(std::shared_ptr<IFxProperty> pNode) = 0;
};

//////////////////////////////////////////////////////////////////////////
struct SColorPty;
class IEffectProperty
{
public:
	virtual ~IEffectProperty() {};

	virtual bool IsDirty() = 0;
	virtual void SetDirty(bool b) = 0;

	virtual void SetDefault() = 0;

	virtual void SetLuminance(float f) = 0;
	virtual float GetLuminance() = 0;

	virtual void SetContrast(float f) = 0;
	virtual float GetContrast() = 0;

	virtual void SetHue(float f) = 0;
	virtual float GetHue() = 0;

	virtual void SetSaturability(float f) = 0;
	virtual float GetSaturability() = 0;

	virtual void SetRed(float f) = 0;
	virtual float GetRed() = 0;

	virtual void SetGreen(float f) = 0;
	virtual float GetGreen() = 0;

	virtual void SetBlue(float f) = 0;
	virtual float GetBlue() = 0;

	virtual void SetAlpha(float f) = 0;
	virtual float GetAlpha() = 0;

	virtual void SetFadeAlpha(const float& fValue) = 0;
	virtual float GetFadeAlpha() = 0;

	virtual SColorPty GetColorProperty() const = 0;
	virtual void SetColorPty(const SColorPty& pty) = 0;

	virtual void SetGlobalSpeaker(const QString& name) = 0;
	virtual QString GetGlobalSpeaker() = 0;

	virtual void SetGlobalVolume(const float& fValue) = 0;
	virtual float GetGlobalVolume() = 0;

	virtual void SetGlobalMute(bool mute) = 0;
	virtual bool GetGlobalMute() = 0;

	virtual void SetGlobalValue(int type, const float& fValue) = 0;
	virtual float GetGlobalValue(int type) = 0;

	virtual void SetBlackScreen(bool blackScreen) = 0;
	virtual bool GetBlackScreen() = 0;

	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
	virtual void LoadDataFromRapidJson(const rapidjson::Value& root) = 0;

	virtual void CopyProperty(std::shared_ptr<IEffectProperty> pNode) = 0;
	virtual void CopyProperty(IEffectProperty* pNode) = 0;
};
//////////////////////////////////////////////////////////////////////////

struct SMediaScrollParam;
struct SMediaPageTurnFxInfo;		//翻页特效
struct SChannelMap;					// 声道映射参数
enum EMediaEndType;

class IShareProperty
{
public:
	virtual ~IShareProperty() {};

	virtual void SetMediaAudioSpeaker(const QString & name) = 0;
	virtual QString GetMediaAudioSpeaker() = 0;

	virtual void SetVolume(float f) = 0;
	virtual float GetVolume() = 0;

	virtual void SetMuted(bool bMuted) = 0;
	virtual bool GetMuted() = 0;

	// 开始播放时间
	virtual __int64 GetStartPlayTime() const = 0;
	virtual void SetStartPlayTime(__int64 time) = 0;

	// 媒体翻页，滚动数据
	virtual void SetControllType(int nType) = 0;
	virtual int GetControllType() = 0;

	virtual int GetPageWidth() = 0;
	virtual void SetPageWidth(int w) = 0;

	virtual int GetPageHeight() = 0;
	virtual void SetPageHeight(int h) = 0;

	virtual int GetPageCount() = 0;
	virtual void SetPageCount(int count) = 0;

	virtual void SetAutoTurnInterval(__int64 nMs) = 0;
	virtual __int64 GetAutoTurnInterval() = 0;

	virtual std::shared_ptr <SMediaScrollParam> GetScrollParam() = 0;
	virtual void SetScrollParam(std::shared_ptr <SMediaScrollParam> pParam) = 0;
	virtual void SetScrollParamData(SMediaScrollParam scrollParam) = 0;

	virtual std::shared_ptr<SMediaPageTurnFxInfo> GetPageTurnFxInfo() = 0;					// 翻页特效
	virtual void SetPageTurnFxInfo(std::shared_ptr<SMediaPageTurnFxInfo> pFxInfo) = 0 ;
	virtual void SetPageTurnFxInfoData(SMediaPageTurnFxInfo TurnFxInfo) = 0;

	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) = 0;
	virtual void LoadDataFromRapidJson(const rapidjson::Value& root) = 0;

	virtual void CopyProperty(std::shared_ptr<IShareProperty> pNode) = 0;

	// 播放设置相关参数置位
	virtual void ResetPlaySetting() = 0;
	virtual void ResetVoiceSetting() = 0;

	// 拼接合集分辨率
	virtual void SetStitchGrpResBaseSize(const QSize &sz) = 0;
	virtual QSize GetStitchGrpResBaseSize() = 0;

	virtual void SetUsedChannelMap(bool bUse) = 0;
	virtual bool GetUsedChannelMap() = 0;

	virtual void SetAudioChannelMap(const SChannelMap& map) = 0;
	virtual SChannelMap GetAudioChannelMap() = 0;
};

// 媒体属性
struct SMediaPty
{
	std::shared_ptr<IEffectProperty> pEffectPty = nullptr;
	std::shared_ptr<IMediaBaseProperty> pBasePty = nullptr;
	std::shared_ptr<IShareProperty> pSharePty = nullptr;
	std::shared_ptr<IFxProperty> pFxPty = nullptr;
};
