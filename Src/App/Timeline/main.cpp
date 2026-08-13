#include "MainWindow.h"
#include "StyleHelper.h"
#include "PreviewGpuPipeline.h"

#include <QApplication>
#include <QStyleFactory>
#include <QIcon>

int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication app(argc, argv);
	app.setApplicationName(QStringLiteral("Timeline"));
	app.setOrganizationName(QStringLiteral("Timeline"));
	app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
	app.setWindowIcon(QIcon(QStringLiteral(":/dark/images/App/Timeline.svg")));

	// 暗色 QSS（DPI 占位 @N$ 已处理）
	InitSystemDpi(QStringLiteral(":/style/dark/stylesheet/style.css"));

	// GPU 管线：D3D Device + 硬解适配器（失败时预览回落软解路径）
	CPreviewGpuPipeline::Instance().InitDevice();

	CMainWindow w;
	w.showMaximized();
	const int code = app.exec();
	CPreviewGpuPipeline::Instance().UninitDevice();
	return code;
}
