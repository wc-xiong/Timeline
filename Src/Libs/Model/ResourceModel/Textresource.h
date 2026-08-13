#pragma once
#include "ResourceBase.h"
#include "Common/MediaCommon.h"
#include <QFont>
#include <QSize>
#include <memory>
#include "ResourceModelGlobal.h"

class RESOURCEMODEL_EXPORT CTextResource :
	public CResourceBase,
	public ITextResource
{
	INIT_ICOPYINTERFACE(CTextResource)
public:
public:
	CTextResource();
	virtual ~CTextResource();
	CTextResource(const CTextResource& other);
	CTextResource& operator= (const CTextResource& other);

	int				GetShowType() override;									//获取文本素材类型
	void			SetShowType(int nTextType) override;						//设置文本素材类型

	//读取
	QString			GetText() override;							//文本内容html
	bool			IsHasBackGround() override;					//是否有背景颜色
	QColor			GetBackColor() override;					//获取背景颜色
	int				GetAlignMent() override;					//获取对齐方式
	QFont			GetTextFont() override;
	QColor			GetTextColor() override;

	//幻彩风格
	virtual int			GetColorTextType() override;

	SColorTextFillParam GetSColorTextFillParam() override;
	SColorTextOutlineParam GetSColorTextOutlineParam() override;
	SColorTextShadeParam GetSColorTextShadeParam() override;
	SColorTextBackgroundParam GetSColorTextBackgroundParam() override;

	void SetSColorTextFillParam(const SColorTextFillParam& node) override {
		m_pColorTextFillParam = node;
	}
	void SetSColorTextOutlineParam(const SColorTextOutlineParam& node) override {
		m_pColorTextOutlineParam = node;
	}
	void SetSColorTextShadeParam(const SColorTextShadeParam& node) override {
		m_pColorTextShadeParam = node;
	}
	void SetSColorTextBackgroundParam(const SColorTextBackgroundParam& node) override {
		m_pColorTextBackgroundParam = node;
	}

	bool GetSColorTextFillParamEnable() override;
	bool GetSColorTextOutlineParamEnable() override;
	bool GetSColorTextShadeParamEnable() override;
	bool GetSColorTextBackgroundParamEnable() override;

	int	GetBackgroundType() override;
	bool IsAnimationText() override;
	//设置
	//文本输入和滚动属性
	void		SetText(const QString& Text) override;					//文本内容
	void		SetAlignMent(int align) override;

	//常规文本
	void		SetHasBackGround(bool b) override;						//设置是否需要背景颜色
	void		SetBackColor(const QColor& backColor) override;			//背景颜色	
	void		SetTextFont(QFont font) override;
	void		SetTextColor(QColor clr) override;

	//幻彩字
	void		SetColorTextType(int nType) override;

	//幻彩字-SetColorTextFillParam-SetCTFParam
	void SetCTFParamEnable(bool bEnable) override;								//填充参数是否可用
	void SetCTFParamFillType(int bType) override;
	void SetCTFParamFillColor(QColor Color) override;
	void SetCTFParamOpacity(double dOpacity) override;

	void SetCTFParamGradientColorNum(int nValue) override;
	void SetCTFParamGradientColor(QColor Color, int nValue) override;
	void SetCTFParamGradientPosition(int nValue, double dPosition) override;

	void SetCTFParamFillTexturechange(QString strPath) override;
	void SetCTFParamImportTexturechange(QVector<QString>& vecStr) override;

	void SetCTFParamGradientDirection(int nType) override;
	void SetCTFParamTextureFillType(int nType) override;
	void SetCTFParamGradientScrollType(int nValue) override;
	void SetCTFParamTextureScrollType(int nValue) override;
	void SetCTFParamScrollSpeed(int nValue) override;

	//幻彩字-SetColorTextOutlineParam-SetCTOParam
	void SetCTOParamEnable(bool bEnable) override;
	void SetCTOParamOutlineColor(QColor Color) override;
	void SetCTOParamThickness(int nValue) override;
	void SetCTOParamOpacity(double dValue) override;

	//幻彩字-SetColorTextShadeParam-SetCTSParam
	void SetCTSParamEnable(bool bEnable) override;
	void SetCTSParamShadeColor(QColor Color) override;
	void SetCTSParamAngle(int nValue) override;
	void SetCTSParamDistance(int nValue) override;
	void SetCTSParamAmbiguity(double dValue) override;

	//幻彩字-SetColorTextBackgroundParam-SetCTBParam
	void SetCTBParamEnable(bool bEnable) override;
	void SetCTSParamBackgroundType(int bType) override;
	void SetCTBParamBackgroundColor(QColor Color) override;
	void SetCTBParamOpacity(double dValue) override;
	void SetCTBParamFillet(double dValue) override;
	void SetCTBParamWidth(double dValue) override;
	void SetCTBParamHeight(double dValue) override;

	void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

protected:
	//文本输入属性
	int			m_nHAlignMent = Qt::AlignLeft;					//对齐方式
	QFont		m_textFont;										//字体样式：字体、加粗、斜体、下划线等
	QString		m_strContent;									//内容
	
	//显示风格
	int			m_nShowType = 0;	

	//一般风格
	QColor		m_BGColor;
	QColor		m_textColor;
	bool		m_bBcakGround = false;

	//幻彩风格
	int			m_nColorTextType = -1;												//幻彩效果标识,主要用于撤销恢复功能
	SColorTextFillParam		m_pColorTextFillParam;				//填充参数
	SColorTextOutlineParam		m_pColorTextOutlineParam;			//描边参数
	SColorTextShadeParam		m_pColorTextShadeParam;				//阴影参数
	SColorTextBackgroundParam	m_pColorTextBackgroundParam;		//背景参数
};

//读取
inline QString		CTextResource::GetText() { return m_strContent; }
inline int			CTextResource::GetShowType() { return m_nShowType; }
inline bool			CTextResource::IsHasBackGround() { return m_bBcakGround; }
inline QColor		CTextResource::GetBackColor() { return m_BGColor; }
inline int			CTextResource::GetAlignMent(){ return m_nHAlignMent; }
inline QFont		CTextResource::GetTextFont(){	return m_textFont;}
inline QColor		CTextResource::GetTextColor(){	return m_textColor;}

inline SColorTextFillParam CTextResource::GetSColorTextFillParam() { return m_pColorTextFillParam; }
inline SColorTextOutlineParam CTextResource::GetSColorTextOutlineParam() { return m_pColorTextOutlineParam; }
inline SColorTextShadeParam CTextResource::GetSColorTextShadeParam() { return m_pColorTextShadeParam; }
inline SColorTextBackgroundParam CTextResource::GetSColorTextBackgroundParam(){return m_pColorTextBackgroundParam;}

inline bool CTextResource::GetSColorTextFillParamEnable() { return m_pColorTextFillParam.bFillEnable; }
inline bool CTextResource::GetSColorTextOutlineParamEnable() { return m_pColorTextOutlineParam.bOutlineEnable; }
inline bool CTextResource::GetSColorTextShadeParamEnable() { return m_pColorTextShadeParam.bShadeEnable; }
inline bool CTextResource::GetSColorTextBackgroundParamEnable() { return m_pColorTextBackgroundParam.bBackgroundEnable; }
inline int	CTextResource::GetColorTextType() { return m_nColorTextType; }

//设置
inline void			CTextResource::SetColorTextType(int nType) {m_nColorTextType = nType; }
inline void			CTextResource::SetText(const QString& Text) { m_strContent = Text; }
inline void			CTextResource::SetBackColor(const QColor& backColor) { m_BGColor = backColor;}
inline void			CTextResource::SetAlignMent(int align) { m_nHAlignMent = align; }
inline void			CTextResource::SetTextFont(QFont font) { m_textFont = font;}
inline void			CTextResource::SetTextColor(QColor clr) { m_textColor = clr;}