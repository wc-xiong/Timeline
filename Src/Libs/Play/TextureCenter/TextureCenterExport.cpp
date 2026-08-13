
#include "TextureCenterExport.h"
#include "TextureTree.h"

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
using namespace TextureCenter;

TEXTURECENTER_EXPORT std::shared_ptr<ITextureTree> TextureCenter::CreateTextureTree(ETextureTreeType type)
{
	switch (type)
	{
	case TextureCenter::ETTT_DX11Texture:
	case TextureCenter::ETTT_Tree:
		return make_shared<TextureTree>(type);
	default:
		assert(0);
		return nullptr;
	}
}
