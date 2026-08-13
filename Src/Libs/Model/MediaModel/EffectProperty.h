#pragma once
#include "MediaModelGlobal.h"
#include "Common/Common/IMediaPropertyInterface.h"
#include "Common/Common/MediaCommon.h"

#include <map>

class MEDIAMODEL_EXPORT CEffectProperty : public IEffectProperty, public std::enable_shared_from_this<CEffectProperty>
{
public:
	CEffectProperty();
	virtual ~CEffectProperty();

	CEffectProperty(const CEffectProperty& data);
	CEffectProperty(std::shared_ptr<IEffectProperty> pdata);
	CEffectProperty(IEffectProperty* pdata);
	CEffectProperty& operator=(const CEffectProperty& data);

public:
	bool IsDirty() override;
	void SetDirty(bool b) override;

	void SetDefault() override;

	void SetLuminance(float f) override;
	float GetLuminance() override;

	void SetContrast(float f) override;
	float GetContrast() override;

	void SetHue(float f) override;
	float GetHue() override;

	void SetSaturability(float f) override;
	float GetSaturability() override;

	void SetRed(float f) override;
	float GetRed() override;

	void SetGreen(float f) override;
	float GetGreen() override;

	void SetBlue(float f) override;
	float GetBlue() override;

	void SetAlpha(float f) override;
	float GetAlpha() override;

	void SetFadeAlpha(const float& fValue) override;
	float GetFadeAlpha() override;

	SColorPty GetColorProperty() const override;
	void SetColorPty(const SColorPty& pty) override;

	void SetGlobalSpeaker(const QString& name) override;
	QString GetGlobalSpeaker() override;

	virtual void SetGlobalVolume(const float& fValue) override;
	virtual float GetGlobalVolume() override;

	virtual void SetGlobalMute(bool mute) override;
	virtual bool GetGlobalMute() override;

	virtual void SetGlobalValue(int type, const float& fValue) override;
	virtual float GetGlobalValue(int type) override;

	virtual void SetBlackScreen(bool blackScreen) override;
	virtual bool GetBlackScreen() override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual void LoadDataFromRapidJson(const rapidjson::Value &root) override;

	virtual void CopyProperty(std::shared_ptr<IEffectProperty> pNode) override;
	virtual void CopyProperty(IEffectProperty* pNode) override;

private:
	SColorPty m_sColorProperty;
	QString m_Speaker;
	float m_fFadeAlpha = 1.0f;	// 场景淡入淡出透明度值
	float m_fVolume; 
	bool  m_bMute = false;
	bool  m_bBlackScreen = false;

	bool m_bDirty = true;

	enum EGlobalEffectType
	{
		EGET_invalid = -1,	    // 无效
		EGET_luminance = 0,		// 亮度
		EGET_contrast,		    // 对比度
		EGET_hue,				// 色相
		EGET_saturability,		// 饱和度
		EGET_red,
		EGET_green,
		EGET_blue,
		EGET_volume,			// 声音
	};

	std::map<EGlobalEffectType, std::function<void(float)> > m_mapSetProperty;
	std::map<EGlobalEffectType, std::function<float()> > m_mapGetProperty;
};

inline bool CEffectProperty::IsDirty()
{
	return m_bDirty;
}

inline void CEffectProperty::SetDirty(bool b)
{
	m_bDirty = b;
}

inline void CEffectProperty::SetLuminance(float f){
	m_bDirty = true;
	m_sColorProperty.luminance = f;
}
inline float CEffectProperty::GetLuminance(){
	return m_sColorProperty.luminance;
}
inline void CEffectProperty::SetContrast(float f){
	m_bDirty = true;
	m_sColorProperty.contrast = f;
}

inline float CEffectProperty::GetContrast(){
	return m_sColorProperty.contrast;
}

inline void CEffectProperty::SetHue(float f){
	m_bDirty = true;
	m_sColorProperty.hue = f;
}

inline float CEffectProperty::GetHue(){
	return m_sColorProperty.hue;
}

inline void CEffectProperty::SetSaturability(float f){
	m_bDirty = true;
	m_sColorProperty.saturability = f;
}

inline float CEffectProperty::GetSaturability(){
	return m_sColorProperty.saturability;
}

inline void CEffectProperty::SetRed(float f){
	m_bDirty = true;
	m_sColorProperty.red = f;
}

inline float CEffectProperty::GetRed(){
	return m_sColorProperty.red;
}

inline void CEffectProperty::SetGreen(float f){
	m_bDirty = true;
	m_sColorProperty.green = f;
}

inline float CEffectProperty::GetGreen(){
	return m_sColorProperty.green;
}

inline void CEffectProperty::SetBlue(float f){
	m_bDirty = true;
	m_sColorProperty.blue = f;
}

inline float CEffectProperty::GetBlue(){
	return m_sColorProperty.blue;
}

inline void CEffectProperty::SetAlpha(float f){
	m_bDirty = true;
	m_sColorProperty.alpha = f;
}

inline float CEffectProperty::GetAlpha(){
	return m_sColorProperty.alpha;
}

inline SColorPty CEffectProperty::GetColorProperty() const{
	return m_sColorProperty;
}

inline void CEffectProperty::SetColorPty(const SColorPty & pty)
{
	m_bDirty = true;
	m_sColorProperty = pty;
}

inline void CEffectProperty::SetGlobalSpeaker(const QString & name)
{
	m_Speaker = name;
}

inline QString CEffectProperty::GetGlobalSpeaker()
{
	return m_Speaker;
}

