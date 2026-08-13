#pragma once
#include "TextureCenterExport.h"

class TextureTree: 
	public TextureCenter::ITextureTree,
	public std::enable_shared_from_this<TextureTree>
{
public:
	explicit TextureTree(TextureCenter::ETextureTreeType type) noexcept;
	TextureTree(const TextureTree&) = delete;
	TextureTree& operator=(const TextureTree&) = delete;
	virtual ~TextureTree();

public:
	TextureCenter::ETextureTreeType GetType() const override;
	STextureTreeNodeParam GetNodeParam() const override;
	TextureCenter::STextureProperty GetTextureProperty() const override;
	std::shared_ptr<STextureFrame> GetDX11Texture() const override;
	std::shared_ptr<STextureRenderParam> GetRenderParam() const override;
	std::vector<std::shared_ptr<ITextureTree>> GetChilds() const override;

	//特效参数
	//std::vector<std::shared_ptr<CFilterCommand>> GetEffectsCommands() const override;	

	std::shared_ptr<ITextureTree> SearchTexture(GUID cellId) override;
	std::shared_ptr<STextClipRenderParam> GetTextRenderParam() const override;

	void SetNodeParam(STextureTreeNodeParam nodeParam) override;
	void SetTextureProperty(TextureCenter::STextureProperty property) override;
	void SetDX11Texture(std::shared_ptr<STextureFrame> pTexture) override;
	void SetRenderParam(std::shared_ptr<STextureRenderParam> pParam) override;
	bool AddChild(std::shared_ptr<ITextureTree> pChild) override;
	bool InsertChild(std::shared_ptr<ITextureTree> pChild) override;

	//特效参数
	//void AddEffectsCommands(const std::shared_ptr<CFilterCommand> pCmd) override;
	void SetTextRenderParam(std::shared_ptr<STextClipRenderParam> pParam) override;

	void ClearFrameParam() override;

protected:
	TextureCenter::ETextureTreeType mType = TextureCenter::ETTT_None;
	STextureTreeNodeParam mTreeNodeParam;
	TextureCenter::STextureProperty mTextureProperty;
	std::shared_ptr<STextureFrame> mTexture = nullptr;
	std::shared_ptr<STextureRenderParam> mRenderParam = nullptr;

	//特效渲染数据，包含转场特效
	//std::vector<std::shared_ptr<CFilterCommand>> mEffectsCmds;
	//文本渲染参数,文本素材时不为空
	std::shared_ptr<STextClipRenderParam> mTextRenderParam = nullptr;

	std::vector<std::shared_ptr<ITextureTree>> mChilds;
};

