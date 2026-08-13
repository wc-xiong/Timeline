#pragma once
#include <chrono>
#include <QUuid>
#include <Windows.h>

enum
{
	IRender_Main = 0,
	IRender_Input,
	IRender_Output,
	IRender_3D,
	IRender_CameraView,

	//最后enum值，后续为多个屏幕交换链索引
	IRender_Screen,
};

class IRender
{
public:
	virtual ~IRender() = default;

	virtual void Render(std::chrono::microseconds ts) = 0;
	virtual bool IsCanRender() = 0;
};


class IMainRender
{
public:
	virtual ~IMainRender() = default;

	virtual bool GetRenderTexHandle(int nDrawType, const QUuid& screenId, HANDLE& hanlde) = 0;
};

