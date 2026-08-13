#include "TestChartResource.h"

#include <iostream>
#include <QTextDocument>

#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"

#include "UICommon/UICommonSizeDefine.h"
#include "Common/ResourceDefine.h"
#include "Common/CommonAboutCustomization.h"
#include "Common/MediaCommon.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTestChartResource::CTestChartResource()
{
	m_nResType = ERT_TestChart;

	//m_szResolution = QSize(331, 60);
	m_szResolution = QSize(m_sTestChartData.baseInfo.m_iWidth, m_sTestChartData.baseInfo.m_iHeight);
	m_nDuration = g_pProjectSetting->LoadDefaultToolDuration();
}

CTestChartResource::CTestChartResource(const CTestChartResource& other)
{
	*this = other;
}

CTestChartResource& CTestChartResource::operator=(const CTestChartResource& other)
{
	*((CResourceBase*)(this)) = other;

	m_sTestChartData = other.m_sTestChartData;
	return *this;
}

CTestChartResource::~CTestChartResource()
{

}

inline void CTestChartResource::SetResolution(QSize sz)
{
	__super::SetResolution(sz);

	m_sTestChartData.baseInfo.m_iWidth = sz.width();
	m_sTestChartData.baseInfo.m_iHeight = sz.height();

	int width = sz.width();
	if (width > MAX_RESOURCE_WIDTH)
	{
		width = MAX_RESOURCE_WIDTH;
	}

	int height = sz.height();
	if (height > MAX_RESOURCE_HEIGHT)
	{
		height = MAX_RESOURCE_HEIGHT;
	}

	float factor = width >= height ? height / 1080.0f : width / 1080.0f;

	STestChartData defaultData;

	SSplineLine splineLine = GetSplineLine();
	splineLine.m_iWidth = defaultData.spline.m_iWidth * factor;
	SetSplineLine(splineLine);

	SCheek cheek = GetCheek();
	cheek.m_iWidth = defaultData.cheek.m_iWidth*factor;
	SetCheek(cheek);

	SCircle circle = GetCircle();
	circle.m_iWidth = defaultData.circle.m_iWidth* factor;
	const int baseRadius = (m_sTestChartData.chartType == TestChartType_Skybox)
		? kSkyboxCircleRadiusDefault
		: defaultData.circle.m_iRadius;
	circle.m_iRadius = static_cast<int>(baseRadius * factor);
	SetCircle(circle);

	SDiagonal diagonal = GetDiagonal();
	diagonal.m_iWidth = defaultData.diagonal.m_iWidth* factor;
	SetDiagonal(diagonal);

	SLogo logo = GetLogo();
	logo.m_iZoom = defaultData.LogoInfo.m_iZoom* factor;

	logo.m_iX = width * 0.5;
	logo.m_iY = height * 0.39;
	SetLogo(logo);
}

void CTestChartResource::SetResolutionKeepProps(QSize sz)
{
	__super::SetResolution(sz);
	m_sTestChartData.baseInfo.m_iWidth = sz.width();
	m_sTestChartData.baseInfo.m_iHeight = sz.height();
}

void CTestChartResource::SetColorMode(const int nType)
{
	m_sTestChartData.colorModeSelect.eColorMode = EColorMode(nType);

	// 平面网格：素材库缩略图跟随颜色模式图标（纯色/渐变/棋盘/色卡）
	// 单鱼眼/双鱼眼/天空盒：与新建测试图一致，固定彩色 TestChart.svg（勿用 Solid Color.png，否则呈橙色 GM 图）
	QPixmap pix;
	if (m_sTestChartData.chartType == TestChartType_PlaneGrid)
	{
		switch (nType)
		{
		case EColorMode::PURECOLOR:
			pix = QPixmap(":/dark/images/ResourceWnd/TestChart/Solid Color.png");
			break;
		case EColorMode::GRADIENT:
			pix = QPixmap(":/dark/images/ResourceWnd/TestChart/Gradient.png");
			break;
		case EColorMode::CHESSBOARD:
			pix = QPixmap(":/dark/images/ResourceWnd/TestChart/Checkerboard.png");
			break;
		case EColorMode::TESTCARD:
			pix = QPixmap(":/dark/images/ResourceWnd/TestChart/Color Card.png");
			break;
		default:
			break;
		}
	}

	if (pix.isNull())
		pix = QPixmap(":/dark/images/ResourceWnd/Thumbmail/Normal/TestChart.svg");

	pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
	SetThumbnail(pix);
}

void CTestChartResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	rapidjson::Value testChartRapid(rapidjson::kObjectType);
	m_sTestChartData.SaveDataToRapidJson(testChartRapid, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_TestChartData, allocator), testChartRapid, allocator);
}

bool CTestChartResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);

	if (!root.IsObject() ||
		!root.HasMember(Xml_ResourceModel_TestChartData) ||
		!root[Xml_ResourceModel_TestChartData].IsObject())
		return false;

	m_sTestChartData.LoadDataFromRapidJson(root[Xml_ResourceModel_TestChartData]);

	QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/TestChart.svg");
	pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
	SetThumbnail(pix);

	return true;
}

QImage CTestChartResource::GetTestChartImage()
{
	return m_image;
}

void CTestChartResource::SetTestChartImage(const QImage & testChartImage)
{
	m_image = testChartImage;
}



