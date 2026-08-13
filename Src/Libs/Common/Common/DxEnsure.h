#pragma once

#include <QApplication>
#include <iostream>

#ifndef DX_LOG_COMPONMENT
#define DX_LOGO_COMPONMENT "DirectX"
#endif // !DX_LOG_COMPONMENT

#define DX_ENSURE(f, ...) DX_CHECK(f, return __VA_ARGS__;)

//std::cout<< DX_LOGO_COMPONMENT " error@" << hrdx << __LINE__<<std::endl; 

#define DX_CHECK(f, ...) \
    do { \
		HRESULT hrdx = f; \
			if (!SUCCEEDED(hrdx)) { \
				std::cout<< DX_LOGO_COMPONMENT " error@" << __LINE__<< " -- "<<std::hex<< hrdx <<std::endl; \
				if(hrdx == DXGI_ERROR_DEVICE_HUNG || hrdx == DXGI_ERROR_DEVICE_REMOVED || hrdx == DXGI_ERROR_DEVICE_RESET) { \
					QApplication::exit(APP_CODE_REBOOT); \
				} \
			} \
    } while (0)