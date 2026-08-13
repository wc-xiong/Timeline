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

TextureTree::TextureTree(TextureCenter::ETextureTreeType type) noexcept
	:mType(type)
{
}

TextureTree::~TextureTree()
{
}

TextureCenter::ETextureTreeType TextureTree::GetType() const
{
	return mType;
}

STextureTreeNodeParam TextureTree::GetNodeParam() const
{
	return mTreeNodeParam;
}

TextureCenter::STextureProperty TextureTree::GetTextureProperty() const
{
	return mTextureProperty;
}

std::shared_ptr<STextureFrame> TextureTree::GetDX11Texture() const
{
	return mTexture;
}

std::shared_ptr<STextureRenderParam> TextureTree::GetRenderParam() const
{
	return mRenderParam;
}

std::vector<std::shared_ptr<TextureCenter::ITextureTree>> TextureTree::GetChilds() const
{
	return mChilds;
}

#if 0
std::vector<std::shared_ptr<CFilterCommand>> TextureTree::GetEffectsCommands() const
{
	return mEffectsCmds;
}
#endif

std::shared_ptr<ITextureTree> TextureTree::SearchTexture(GUID cellId)
{
	switch (mType)
	{
	case TextureCenter::ETTT_Tree:
	{
		for (auto& item: mChilds)
		{
			auto pTemp = item->SearchTexture(cellId);
			if (pTemp != nullptr)
				return pTemp;
		}
		break;
	}
	case TextureCenter::ETTT_DX11Texture:
	{
		if (mTreeNodeParam.mCellId == cellId)
			return shared_from_this();
		break;
	}
	default:
		assert(0);
		return nullptr;
	}

	return nullptr;
}

std::shared_ptr<STextClipRenderParam> TextureTree::GetTextRenderParam() const
{
	return mTextRenderParam;
}

void TextureTree::SetNodeParam(STextureTreeNodeParam nodeParam)
{
	mTreeNodeParam = nodeParam;
}

void TextureTree::SetTextureProperty(TextureCenter::STextureProperty property)
{
	//ASSERT_HANDLER(mType == TextureCenter::ETTT_DX11Texture, return);

	mTextureProperty = move(property);
}

void TextureTree::SetDX11Texture(std::shared_ptr<STextureFrame> pTexture)
{
	//ASSERT_HANDLER(mType == TextureCenter::ETTT_DX11Texture, return);

	mTexture = pTexture;
}

void TextureTree::SetRenderParam(std::shared_ptr<STextureRenderParam> pParam)
{
	//ASSERT_HANDLER(mType == TextureCenter::ETTT_DX11Texture, return);

	mRenderParam = pParam;
}

bool TextureTree::AddChild(std::shared_ptr<ITextureTree> pChild)
{
	//ASSERT_HANDLER(pChild != nullptr, return false);
	//ASSERT_HANDLER(mType == TextureCenter::ETTT_Tree, return false);

	mChilds.push_back(pChild);

	return true;
}

bool TextureTree::InsertChild(std::shared_ptr<ITextureTree> pChild)
{
	//ASSERT_HANDLER(pChild != nullptr, return false);
	//ASSERT_HANDLER(mType == TextureCenter::ETTT_Tree, return false);

	mChilds.insert(mChilds.begin(), pChild);

	return true;
}

#if 0
void TextureTree::AddEffectsCommands(const std::shared_ptr<CFilterCommand> pCmd)
{
	ASSERT_HANDLER(pCmd != nullptr, return);

	mEffectsCmds.push_back(pCmd);
}
#endif

void TextureTree::SetTextRenderParam(std::shared_ptr<STextClipRenderParam> pParam)
{
	//ASSERT_HANDLER(mType == TextureCenter::ETTT_DX11Texture, return);
	//ASSERT_HANDLER(pParam != nullptr, return);

	mTextRenderParam = pParam;
}

void TextureTree::ClearFrameParam()
{
	mRenderParam = nullptr;
	//mEffectsCmds.clear();
	mTextRenderParam = nullptr;

	for (auto& item : mChilds)
		item->ClearFrameParam();
}
