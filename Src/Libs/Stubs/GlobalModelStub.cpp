#include "Model/GlobalModel/GlobalModel.h"

CGlobalModel::CGlobalModel() = default;
CGlobalModel::~CGlobalModel() = default;

CGlobalModel* CGlobalModel::GetInstance()
{
	static CGlobalModel instance;
	return &instance;
}

void CGlobalModel::InitGlobalSetting() {}
bool CGlobalModel::LoadOrCreateLocalMachineInfo() { return true; }
