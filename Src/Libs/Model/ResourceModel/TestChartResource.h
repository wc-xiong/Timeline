#pragma once
#include "ResourceBase.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/TestChartResDefine.h"
#include <QFont>
#include <QSize>
#include <memory>

class CTestChartResource :
	public CResourceBase,
	public ITestChartResource
{
	INIT_ICOPYINTERFACE(CTestChartResource)
public:
public:
	CTestChartResource();
	~CTestChartResource();
	CTestChartResource(const CTestChartResource& other);
	CTestChartResource& operator= (const CTestChartResource& other);

	//读取
	virtual SBaseInfo GetBaseInfo();
	virtual SColorModeSelect GetColorModeSelect();
	virtual SOrder GetOrder();
	virtual SSplineLine GetSplineLine();
	virtual SCheek GetCheek();
	virtual SCircle GetCircle();
	virtual SDiagonal GetDiagonal();
	virtual SResolution GetResolutionInfo();
	virtual SBoxNum GetBoxNum();
	virtual SLogo GetLogo();
	virtual STextInfo GetTextInfo();

	virtual STestChartData GetTestChartParam();     //获取测试图参数


	//设置
	virtual void SetBaseInfo(const SBaseInfo & baseInfo);
	virtual void SetColorModeSelect(const SColorModeSelect & colorMode);
	virtual void SetOrder(const SOrder & order);
	virtual void SetSplineLine(const SSplineLine & splineLine);
	virtual void SetCheek(const SCheek & cheek);
	virtual void SetCircle(const SCircle & circle);
	virtual void SetDiagonal(const SDiagonal & diagonal);
	virtual void SetResolutionInfo(const SResolution & resolution);
	virtual void SetBoxNum(const SBoxNum & boxNum);
	virtual void SetLogo(const SLogo & logo);
	virtual void SetTextInfo(const STextInfo & textInfo);
	virtual void SetTestChartParam(const STestChartData & testChartParam);

	virtual void SetResolution(QSize sz)override;
	virtual void SetResolutionKeepProps(QSize sz) override;
	virtual void SetColorMode(const int nType);

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

	virtual QImage GetTestChartImage() override;//获取测试图     	
	virtual void SetTestChartImage(const QImage & testChartImage) override;//设置测试图

protected:
	QImage m_image;						// 当前绘制出来的测试图
	STestChartData   m_sTestChartData; //存储数据的数据结构
};

//读取
inline SBaseInfo CTestChartResource::GetBaseInfo() { return m_sTestChartData.baseInfo; }
inline SColorModeSelect CTestChartResource::GetColorModeSelect() { return m_sTestChartData.colorModeSelect; }
inline SOrder CTestChartResource::GetOrder() { return m_sTestChartData.order; }
inline SSplineLine CTestChartResource::GetSplineLine() { return m_sTestChartData.spline; }
inline SCheek CTestChartResource::GetCheek() { return m_sTestChartData.cheek; }
inline SCircle CTestChartResource::GetCircle() { return m_sTestChartData.circle; }
inline SDiagonal CTestChartResource::GetDiagonal() { return m_sTestChartData.diagonal; }
inline SResolution CTestChartResource::GetResolutionInfo() { return m_sTestChartData.resolution; }
inline SBoxNum CTestChartResource::GetBoxNum() { return m_sTestChartData.boxNum; }
inline SLogo CTestChartResource::GetLogo() { return m_sTestChartData.LogoInfo; }
inline STextInfo CTestChartResource::GetTextInfo() { return m_sTestChartData.textInfo; }

inline STestChartData CTestChartResource::GetTestChartParam() { return m_sTestChartData; }


//设置
inline void CTestChartResource::SetBaseInfo(const SBaseInfo & baseInfo)
{
	m_sTestChartData.baseInfo = baseInfo;
}

inline void CTestChartResource::SetColorModeSelect(const SColorModeSelect & colorMode) { m_sTestChartData.colorModeSelect = colorMode; }
inline void CTestChartResource::SetOrder(const SOrder & order) { m_sTestChartData.order = order; }
inline void CTestChartResource::SetSplineLine(const SSplineLine & splineLine) { m_sTestChartData.spline = splineLine; }
inline void CTestChartResource::SetCheek(const SCheek & cheek) { m_sTestChartData.cheek = cheek; }
inline void CTestChartResource::SetCircle(const SCircle & circle) { m_sTestChartData.circle = circle; }
inline void CTestChartResource::SetDiagonal(const SDiagonal & diagonal) { m_sTestChartData.diagonal = diagonal; }
inline void CTestChartResource::SetResolutionInfo(const SResolution & resolution) { m_sTestChartData.resolution = resolution; }
inline void CTestChartResource::SetBoxNum(const SBoxNum & boxNum) { m_sTestChartData.boxNum = boxNum; }
inline void CTestChartResource::SetLogo(const SLogo & logo) { m_sTestChartData.LogoInfo = logo; }
inline void CTestChartResource::SetTextInfo(const STextInfo & textInfo) { m_sTestChartData.textInfo = textInfo; }
inline void CTestChartResource::SetTestChartParam(const STestChartData & testChartParam) { m_sTestChartData = testChartParam; }
