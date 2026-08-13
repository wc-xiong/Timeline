#pragma once

enum EFpsComboIndex
{
	FPSINDEX_24 = 0,
	FPSINDEX_25,
	FPSINDEX_2997DF,
	FPSINDEX_30,
	FPSINDEX_60,
};

const std::map<int, std::pair<float, QString>> FpsMap =
{
	{FPSINDEX_24, std::make_pair(24.0f, QString("24 FPS"))},
	{FPSINDEX_25, std::make_pair(25.0f, QString("25 FPS"))},
	{FPSINDEX_2997DF, std::make_pair(29.97f, QString("29.97 DF FPS"))},
	{FPSINDEX_30, std::make_pair(30.0f, QString("30 FPS"))},
	{FPSINDEX_60, std::make_pair(60.0f, QString("60 FPS"))},
};
