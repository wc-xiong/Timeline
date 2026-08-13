#pragma once
#include <memory>
#include <vector>
#include <chrono>
#include "TextureCenterGlobal.h"
#include "Common/Common/IRenderParamDefine.h"

// 填充纹理数据，及渲染参数

namespace TextureCenter
{
	enum ETextureTreeType
	{
		ETTT_None, // 非法
		ETTT_DX11Texture, // 纹理实例
		ETTT_Tree, // 纹理树，需要递归才能获取到纹理实例
	};

	struct STextureProperty
	{
		// 纹理的时间戳
		std::chrono::microseconds mPts;
		// 纹理的帧号
		__int64 mFrameIndex = 0;
	};

	struct ITextureTree
	{
		virtual ETextureTreeType GetType() const = 0;
		virtual STextureTreeNodeParam GetNodeParam() const = 0;
		virtual STextureProperty GetTextureProperty() const = 0;
		virtual std::shared_ptr<STextureFrame> GetDX11Texture() const = 0;
		virtual std::shared_ptr<STextureRenderParam> GetRenderParam() const = 0;
		virtual std::vector<std::shared_ptr<ITextureTree>> GetChilds() const = 0;

		//特效参数
		//virtual std::vector<std::shared_ptr<CFilterCommand>> GetEffectsCommands() const = 0;	
		virtual std::shared_ptr<ITextureTree> SearchTexture(GUID cellId) = 0;
		virtual std::shared_ptr<STextClipRenderParam> GetTextRenderParam() const = 0;

		virtual void SetNodeParam(STextureTreeNodeParam nodeParam) = 0;
		virtual void SetTextureProperty(STextureProperty property) = 0;
		virtual void SetDX11Texture(std::shared_ptr<STextureFrame> pTexture) = 0;
		virtual void SetRenderParam(std::shared_ptr<STextureRenderParam> pParam) = 0;
		virtual bool AddChild(std::shared_ptr<ITextureTree> pChild) = 0;
		virtual bool InsertChild(std::shared_ptr<ITextureTree> pChild) = 0;

		//特效参数
		//virtual void AddEffectsCommands(const std::shared_ptr<CFilterCommand> pCmd) = 0;

		virtual void SetTextRenderParam(std::shared_ptr<STextClipRenderParam> pParam) = 0;

		// 清理特效命令
		virtual void ClearFrameParam() = 0;
	};

	TEXTURECENTER_EXPORT std::shared_ptr<ITextureTree> CreateTextureTree(ETextureTreeType type);
}