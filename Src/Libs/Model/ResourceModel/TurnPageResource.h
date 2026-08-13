#pragma once
#include "Common/Common/IResourceInterface.h"


class CTurnPageResource :
	public ITurnPageResource
{
public:
	CTurnPageResource();
	virtual ~CTurnPageResource();
	CTurnPageResource(const CTurnPageResource& other);
	CTurnPageResource& operator=(const CTurnPageResource& other);

	void SetCurPage(int nPage) override;
	int GetCurPage() override;
protected:
	std::atomic<int> m_nCurPage = 0;
};
