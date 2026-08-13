#pragma once
#include "Common/Common/ICopyInterface.h"
#include <memory>

class IHistoryStackCallbacker
{
public:
	virtual ~IHistoryStackCallbacker() {}

	virtual void OnStackChanged() = 0;
};

class IHistoryStack
{
public:
	virtual ~IHistoryStack(){}

	//添加监视
	virtual	void AddWatcher(IHistoryStackCallbacker *pWatcher) = 0;

	//删除监视
	virtual	void RemoveWatcher(IHistoryStackCallbacker *pWatcher) = 0;

	//能否撤销
	virtual bool CanUndo() = 0;

	//能否恢复
	virtual bool CanRedo() = 0;

	//撤销
	virtual bool Undo(ICopyInterface** ppDst) = 0;

	//恢复
	virtual bool Redo(ICopyInterface** ppDst) = 0;

	//记录
	virtual void Save(ICopyInterface* pSrc) = 0;
	//记录
	virtual void Save(std::shared_ptr<ICopyInterface> pSrc) = 0;

	//当前节点
	virtual bool GetCurNode(ICopyInterface** ppDst) = 0;

	//清空记录
	virtual void Clear() = 0;
};