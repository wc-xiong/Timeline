#include "FxFactor.h"

#include "Common/UICommon/UITranslate.h"
#include "Common/CommonRapidJson.h"
#include <QJsonDocument>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

std::shared_ptr<SFxFactor> CreateFxFactor(int type)
{
	switch (type)
	{
	case Blur:
		return  std::shared_ptr<SFxFactor>(new SBlurFactor());
	case Sharpen:
		return  std::shared_ptr<SFxFactor>(new SSharpenFactor());
	case Emboss:
		return  std::shared_ptr<SFxFactor>(new SEmbossFactor());
	case Denoise:
		return  std::shared_ptr<SFxFactor>(new SDenoiseFactor());
	case Frame:
		return  std::shared_ptr<SFxFactor>(new SFrameFactor());
	case Feather:
		return  std::shared_ptr<SFxFactor>(new SFeatherFactor());
	case Keying:
		return  std::shared_ptr<SFxFactor>(new SKeyingFactor());
	case Blink:
		return  std::shared_ptr<SFxFactor>(new SBlinkFactor());
	case RGBA:
		return  std::shared_ptr<SFxFactor>(new SRGBFactor());
	case Shake:
		return  std::shared_ptr<SFxFactor>(new SShakeFactor());
	case Cut:
		return  std::shared_ptr<SFxFactor>(new SCropFactor());
	case Filter:
		return  std::shared_ptr<SFxFactor>(new SFilterFactor());
	case Complement:
	{
		auto p = std::shared_ptr<SFxFactor>(new SFxFactor());
		p->type = type;
		return p;
	}
	case Scroll:
		return  std::shared_ptr<SFxFactor>(new SScrollFactor());
	case Mosaic:
		return  std::shared_ptr<SFxFactor>(new SMosaicFactor());
	case Shadow:
		return  std::shared_ptr<SFxFactor>(new SShadowFactor());
	case HSV:
		return  std::shared_ptr<SFxFactor>(new SClrWheelFactor());
	case Spherize:
		return  std::shared_ptr<SFxFactor>(new SSpherizeFactor());
	case Twirl:
		return  std::shared_ptr<SFxFactor>(new STwirlFactor());
	case Mirror:
		return  std::shared_ptr<SFxFactor>(new SKaleidoscopeFactor());
	case Nightvision:
		return  std::shared_ptr<SFxFactor>(new SNightViewFactor());
	case Marquees:
		return  std::shared_ptr<SFxFactor>(new SMarqueesFactor());
	case Mask:
		return  std::shared_ptr<SFxFactor>(new SMaskFactor());
	default:
		return  std::shared_ptr<SFxFactor>(new SFxFactor());
	}
}

CFxFactor::CFxFactor(int type)
{
	m_type = type;
	m_pdata = CreateFxFactor(m_type);
}

CFxFactor::~CFxFactor()
{

}

CFxFactor::CFxFactor(const CFxFactor & data)
{
	m_type = data.m_type;
	SetData(data.m_pdata);
}

CFxFactor & CFxFactor::operator=(const CFxFactor & data)
{
	if (this == &data)
		return *this;

	m_type = data.m_type;
	SetData(data.m_pdata);

	return *this;
}

CFxFactor& CFxFactor::operator=(std::shared_ptr<IFxFactor> pdata)
{
	if (this == pdata.get())
		return *this;

	m_type = pdata->GetType();
	SetData(pdata->GetData());

	return *this;
}

CFxFactor::CFxFactor(std::shared_ptr<IFxFactor> pdata)
{
	m_type = pdata->GetType();
	m_pdata = CreateFxFactor(m_type);

	SetData(pdata->GetData());
}

int CFxFactor::GetType()
{
	return m_type;
}

void CFxFactor::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	root.SetObject();
	root.AddMember("type", m_type, allocator);
	root.AddMember("name", rapidjson::Value(m_name.toUtf8().constData(), allocator), allocator);
	rapidjson::Document dataDoc;
	QJsonObject dataObj = m_pdata->SaveDataToJson();
	dataDoc.Parse(dataObj.isEmpty() ? "{}" : QJsonDocument(dataObj).toJson(QJsonDocument::Compact).constData());
	if (!dataDoc.HasParseError() && dataDoc.IsObject())
	{
		rapidjson::Value dataValue(rapidjson::kObjectType);
		dataValue.CopyFrom(dataDoc, allocator);
		root.AddMember("data", dataValue, allocator);
	}
	else
	{
		rapidjson::Value dataValue(rapidjson::kObjectType);
		root.AddMember("data", dataValue, allocator);
	}
}

void CFxFactor::LoadDataFromRapidJson(const rapidjson::Value &root)
{
	if (!root.IsObject())
		return;
	if (root.HasMember("type") && root["type"].IsInt())
		m_type = root["type"].GetInt();
	if (root.HasMember("name") && root["name"].IsString())
		m_name = QString::fromUtf8(root["name"].GetString());
	if (root.HasMember("data") && root["data"].IsObject())
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		root["data"].Accept(writer);
		QJsonParseError err;
		QJsonDocument doc = QJsonDocument::fromJson(QByteArray(buffer.GetString(), static_cast<int>(buffer.GetSize())), &err);
		if (err.error == QJsonParseError::NoError && doc.isObject())
			m_pdata->LoadDataFromJson(doc.object());
	}
}

std::shared_ptr<SFxFactor> CFxFactor::GetData()
{
	return m_pdata;
}

QString CFxFactor::GetName()
{
	switch (m_type)
	{
	case Blur:
		m_name = CFxPropertyTranslate::Blur();
		break;
	case Sharpen:
		m_name = CFxPropertyTranslate::Sharpen();
		break;
	case Emboss:
		m_name = CFxPropertyTranslate::Emboss();
		break;
	case Denoise:
		m_name = CFxPropertyTranslate::Denoise();
		break;
	case Frame:
		m_name = CFxPropertyTranslate::Frame();
		break;
	case Feather:
		m_name = CFxPropertyTranslate::Feather();
		break;
	case Keying:
		m_name = CFxPropertyTranslate::Keying();
		break;
	case Blink:
		m_name = CFxPropertyTranslate::Blink();
		break;
	case Shake:
		m_name = CFxPropertyTranslate::Shake();
		break;
	case Cut:
		m_name = CFxPropertyTranslate::Cut();
		break;
	case Filter:
		m_name = CFxPropertyTranslate::Filter();
		break;
	case Complement:
		m_name = CFxPropertyTranslate::Complement();
		break;
	case Scroll:
		m_name = CFxPropertyTranslate::Scroll();
		break;
	case Mosaic:
		m_name = CFxPropertyTranslate::Mosaic();
		break;
	case Shadow:
		m_name = CFxPropertyTranslate::Shadow();
		break;
	case HSV:
		m_name = CFxPropertyTranslate::HSV();
		break;
	case Spherize:
		m_name = CFxPropertyTranslate::Spherize();
		break;
	case Twirl:
		m_name = CFxPropertyTranslate::Twirl();
		break;
	case Mirror:
		m_name = CFxPropertyTranslate::Mirror();
		break;
	case Nightvision:
		m_name = CFxPropertyTranslate::Nightvision();
		break;
	case Marquees:
		m_name = CFxPropertyTranslate::Marquees();
		break;
	case Mask:
		m_name = CFxPropertyTranslate::Mask();
		break;
	default:
		m_name = "";
		break;
	}

	return m_name;
}

void CFxFactor::SetData(std::shared_ptr<SFxFactor> factor)
{
	switch (m_type)
	{
	case Blur:
		*dynamic_cast<SBlurFactor*>(m_pdata.get()) = *dynamic_cast<SBlurFactor*>(factor.get());
		break;
	case Sharpen:
		*dynamic_cast<SSharpenFactor*>(m_pdata.get()) = *dynamic_cast<SSharpenFactor*>(factor.get());
		break;
	case Emboss:
		*dynamic_cast<SEmbossFactor*>(m_pdata.get()) = *dynamic_cast<SEmbossFactor*>(factor.get());
		break;
	case Denoise:
		*dynamic_cast<SDenoiseFactor*>(m_pdata.get()) = *dynamic_cast<SDenoiseFactor*>(factor.get());
		break;
	case Frame:
		*dynamic_cast<SFrameFactor*>(m_pdata.get()) = *dynamic_cast<SFrameFactor*>(factor.get());
		break;
	case Feather:
		*dynamic_cast<SFeatherFactor*>(m_pdata.get()) = *dynamic_cast<SFeatherFactor*>(factor.get());
		break;
	case Keying:
		*dynamic_cast<SKeyingFactor*>(m_pdata.get()) = *dynamic_cast<SKeyingFactor*>(factor.get());
		break;
	case Blink:
		*dynamic_cast<SBlinkFactor*>(m_pdata.get()) = *dynamic_cast<SBlinkFactor*>(factor.get());
		break;
	case RGBA:
		*dynamic_cast<SRGBFactor*>(m_pdata.get()) = *dynamic_cast<SRGBFactor*>(factor.get());
		break;
	case Shake:
		*dynamic_cast<SShakeFactor*>(m_pdata.get()) = *dynamic_cast<SShakeFactor*>(factor.get());
		break;
	case Cut:
		*dynamic_cast<SCropFactor*>(m_pdata.get()) = *dynamic_cast<SCropFactor*>(factor.get());
		break;
	case Filter:
		*dynamic_cast<SFilterFactor*>(m_pdata.get()) = *dynamic_cast<SFilterFactor*>(factor.get());
		break;
	case Complement:
		*(m_pdata.get()) = *(factor.get());
		break;
	case Scroll:
		*dynamic_cast<SScrollFactor*>(m_pdata.get()) = *dynamic_cast<SScrollFactor*>(factor.get());
		break;
	case Mosaic:
		*dynamic_cast<SMosaicFactor*>(m_pdata.get()) = *dynamic_cast<SMosaicFactor*>(factor.get());
		break;
	case Shadow:
		*dynamic_cast<SShadowFactor*>(m_pdata.get()) = *dynamic_cast<SShadowFactor*>(factor.get());
		break;
	case HSV:
		*dynamic_cast<SClrWheelFactor*>(m_pdata.get()) = *dynamic_cast<SClrWheelFactor*>(factor.get());
		break;
	case Spherize:
		*dynamic_cast<SSpherizeFactor*>(m_pdata.get()) = *dynamic_cast<SSpherizeFactor*>(factor.get());
		break;
	case Twirl:
		*dynamic_cast<STwirlFactor*>(m_pdata.get()) = *dynamic_cast<STwirlFactor*>(factor.get());
		break;
	case Mirror:
		*dynamic_cast<SKaleidoscopeFactor*>(m_pdata.get()) = *dynamic_cast<SKaleidoscopeFactor*>(factor.get());
		break;
	case Nightvision:
		*dynamic_cast<SNightViewFactor*>(m_pdata.get()) = *dynamic_cast<SNightViewFactor*>(factor.get());
		break;
	case Marquees:
		*dynamic_cast<SMarqueesFactor*>(m_pdata.get()) = *dynamic_cast<SMarqueesFactor*>(factor.get());
		break;
	case Mask:
		*dynamic_cast<SMaskFactor*>(m_pdata.get()) = *dynamic_cast<SMaskFactor*>(factor.get());
		break;
	default:
		*(m_pdata.get()) = *(factor.get());
		break;
	}
}