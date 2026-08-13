
#include "DecoderControlExport.h"
#include "DecoderManager.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace std;
using namespace DecoderControl;

DECODERCONTROL_EXPORT std::shared_ptr<IManager> DecoderControl::CreateManager(EDecodeManager playMode)
{
	auto pDecodeManager = make_shared<DecoderManager>();
	pDecodeManager->UpdatePlayMode(playMode);

	return pDecodeManager;
}
