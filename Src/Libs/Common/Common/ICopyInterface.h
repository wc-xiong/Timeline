#pragma once
#include "CommonGlobal.h"

class ICopyInterface
{
public:
	virtual ~ICopyInterface() {}

	virtual ICopyInterface* Copy() = 0;

	virtual void CopyTo(ICopyInterface* p) = 0;
};

#define INIT_ICOPYINTERFACE(T) \
public: \
	virtual ICopyInterface* Copy() { return new T(*this); }  \
	virtual void CopyTo(ICopyInterface* p) { *dynamic_cast<T*>(p) = *this; }
