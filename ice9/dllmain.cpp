#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved){

	switch (ul_reason_for_call){

    case DLL_PROCESS_ATTACH: {
        ice9().instance().on_thread_attach(hModule, ul_reason_for_call, lpReserved);
        break;
    }
    case DLL_THREAD_ATTACH: {
        ice9().instance().on_thread_attach(hModule, ul_reason_for_call, lpReserved);
        break;
    }
    case DLL_THREAD_DETACH: {
        ice9().instance().on_thread_detach(hModule, ul_reason_for_call, lpReserved);
        break;
    }
    case DLL_PROCESS_DETACH: {
        ice9().instance().on_thread_detach(hModule, ul_reason_for_call, lpReserved);
        break;
    }
                        
    default:break;
	}
	return TRUE;
}

