#include "TestChartPlayer.h"
#include "Model/ResourceModel/ResourceModel.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Common/Common/IResourceInterface.h"
#include "Common/Common/MediaCommon.h"
#include "Model/ResourceModel/TestChartResource.h"
#include "TestChartPainter.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTestChartPlayer::CTestChartPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx)
	: CToolPlayer(pResBase, ctx)
{
	m_pTestChartPainter = new CTestChartPainter(false);
	m_printImage.load(":/dark/images/ResourceWnd/TestChart/WaterMark.png");
}

CTestChartPlayer::~CTestChartPlayer()
{
	// 必须先停工作线程再删 painter，否则线程仍在 DrawImage 会 UAF（堆栈常落在 Qt5Guid）
	StopUpdateThread();

	if (m_pTestChartPainter)
	{
		delete m_pTestChartPainter;
		m_pTestChartPainter = nullptr;
	}
}

bool CTestChartPlayer::DrawImage()
{
	if (m_bExit.load())
		return false;

	// 撤销/恢复后 ResourceModel 可能复用或替换节点，始终按 resId 取当前资源，避免画到旧 chartType
	if (g_pResourceModel)
	{
		auto pLatest = g_pResourceModel->GetResourceNode(m_ctx.resId);
		if (pLatest)
			m_pResBase = pLatest;
	}

	std::shared_ptr<ITestChartResource> testChartNode = std::dynamic_pointer_cast<ITestChartResource>(m_pResBase);
	if (!testChartNode || !m_pTestChartPainter)
		return false;

	int width = m_pResBase->GetResolution().width();
	if (width > MAX_RESOURCE_WIDTH)
		width = MAX_RESOURCE_WIDTH;

	int height = m_pResBase->GetResolution().height();
	if (height > MAX_RESOURCE_HEIGHT)
		height = MAX_RESOURCE_HEIGHT;

	m_szMedia = QSize(width, height);

	STestChartData data = testChartNode->GetTestChartParam();
	data.baseInfo.m_iWidth = width;
	data.baseInfo.m_iHeight = height;
	m_pTestChartPainter->SetTestChartData(data);
	m_pTestChartPainter->DrawImage(width, height, m_image);

	return CBasePlayer::UpdateImageToData(m_image);
}
