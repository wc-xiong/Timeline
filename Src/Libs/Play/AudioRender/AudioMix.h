#pragma once

#include "Common/AudioCommon.h"
#include <vector>

bool MixAudio(BYTE* stream, int len, std::vector<SAudioPlayData>& datas, const SAudioPlayerProp &prop);

