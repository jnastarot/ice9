#pragma once




int WINAPI client_load_library_handler(SWH_DATA * data) {

#ifdef ICE9_DEBUG
    if (data->lpDllPath.Buffer) {
    //    wprintf(L"client_load_library_handler [%s]\n", data->lpDllPath.Buffer);
    }
#endif
    return 1;

    /*
    you can add logic there 
    */

    if (data->lpDllPath.Buffer) {
        ice9().instance().on_dll_attach(data->lpDllPath.Buffer);
    }
    return _ClientLoadLibrary(ice9().instance().get_client_load_library())(data);
}

VOID WINAPI dll_notify_handler(ULONG NotificationReason,PLDR_DLL_NOTIFICATION_DATA NotificationData,PVOID Context) {

    switch (NotificationReason) {
        case LDR_DLL_NOTIFICATION_REASON_LOADED: {
            ice9().instance().on_dll_attach(NotificationData->Loaded.FullDllName->Buffer);
            break;
        }

        case LDR_DLL_NOTIFICATION_REASON_UNLOADED: {
            ice9().instance().on_dll_detach(NotificationData->Loaded.FullDllName->Buffer);
            break;
        }
    }
}


inline LPVOID * get_apfn_dispatch() {
#ifdef _M_IX86 
    return (LPVOID *)*(DWORD *)(__readfsdword(0x30) + 0x2C);
#else
    return (LPVOID *)*(DWORD64 *)(__readgsqword(0x60) + 0x58);
#endif
}


void ice9::set_dll_hook() {
#ifdef ICE9_DLL_MONITOR_ENABLED
    _LdrRegisterDllNotification LdrRegisterDllNotification = (_LdrRegisterDllNotification)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "LdrRegisterDllNotification"
    );

    if (LdrRegisterDllNotification) {
        LdrRegisterDllNotification(0, dll_notify_handler, 0, (void**)&dll_notify_cookie);
    }
#endif

#ifdef ICE9_PREVENT_WINDOW_HOOK_DLL_ENABLED
    DWORD oldprot;
    LPVOID * apfnDispatch = get_apfn_dispatch();
    if (apfnDispatch) {
        o_ClientLoadLibrary = (BYTE*)apfnDispatch[__ClientLoadLibrary_idx];

        VirtualProtect(apfnDispatch, 117 * sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldprot);
        apfnDispatch[__ClientLoadLibrary_idx] = client_load_library_handler;
        VirtualProtect(apfnDispatch, 117 * sizeof(LPVOID), oldprot, &oldprot);
    }
#endif
}

void ice9::unset_dll_hook() {
#ifdef ICE9_DLL_MONITOR_ENABLED
    _LdrUnregisterDllNotification LdrUnregisterDllNotification = (_LdrUnregisterDllNotification)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "LdrUnregisterDllNotification"
    );

    if (LdrUnregisterDllNotification) {
        LdrUnregisterDllNotification(this->dll_notify_cookie);
    }
#endif

#ifdef ICE9_PREVENT_WINDOW_HOOK_DLL_ENABLED
    DWORD oldprot;
    LPVOID * apfnDispatch = get_apfn_dispatch();
    if (apfnDispatch && o_ClientLoadLibrary) {
        VirtualProtect(apfnDispatch, 117 * sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldprot);
        apfnDispatch[__ClientLoadLibrary_idx] = o_ClientLoadLibrary;
        VirtualProtect(apfnDispatch, 117 * sizeof(LPVOID), oldprot, &oldprot);
    }
#endif
}

void ice9::on_dll_attach(std::wstring dll_path) {
#ifdef ICE9_DEBUG
    wprintf(L"dll attached [%s]\n", dll_path.c_str());
#endif

}


void ice9::on_dll_detach(std::wstring dll_path) {
#ifdef ICE9_DEBUG
    wprintf(L"dll detached [%s]\n", dll_path.c_str());
#endif

}