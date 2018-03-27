#include "stdafx.h"
#include "ice9.h"

bool is_initiated = false;
ice9 * g_ice9 = 0;

ice9::ice9(){
    if (!is_initiated) {
        is_initiated = true;
        g_ice9 = new ice9();
       
#ifdef ICE9_DEBUG  
        if (AllocConsole()) { freopen("CONOUT$", "w", stdout); setlocale(LC_ALL, "RUS"); }
        Sleep(500);
#endif

        g_ice9->do_init_ice9();
        return;
    }
}


ice9::~ice9(){
    //not used
}

void ice9::do_init_ice9() {
    main_module = GetModuleHandle(0);


#ifdef _M_IX86
    IsWow64Process(GetCurrentProcess(), &is_64);
#else
    is_64 = true;
#endif

    funcs.push_back((BYTE*)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationThread"));
    funcs.push_back((BYTE*)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation"));
    funcs.push_back((BYTE*)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryObject"));

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    HMODULE kernelbase = GetModuleHandleA("kernelbase.dll");

    suspected_funcs.push_back((BYTE*)GetProcAddress(kernel32, "LoadLibraryA"));
    suspected_funcs.push_back((BYTE*)GetProcAddress(kernel32, "LoadLibraryW"));
    suspected_funcs.push_back((BYTE*)GetProcAddress(kernel32, "FreeLibrary"));

    if (kernelbase) {
        suspected_funcs.push_back((BYTE*)GetProcAddress(kernelbase, "LoadLibraryA"));
        suspected_funcs.push_back((BYTE*)GetProcAddress(kernelbase, "LoadLibraryW"));
        suspected_funcs.push_back((BYTE*)GetProcAddress(kernelbase, "FreeLibrary"));
    }

#ifdef ICE9_WINDOW_HOOK_EXPERIMENTAL_ENABLED
    set_window_hook_exp();
#endif
    set_dll_hook();

}


ice9 &ice9::instance() {
    return *g_ice9;
}

BYTE * ice9::get_dispatch_client_message_a() {
    return o_DispatchClientMessage_A;
}
BYTE * ice9::get_dispatch_client_message_w() {
    return o_DispatchClientMessage_W;
}
BYTE * ice9::get_client_load_library() {
    return o_ClientLoadLibrary;
}

std::vector<msg_manager>& ice9::get_windows() {
    return windows;
}
std::vector<unsigned int>& ice9::get_threads() {
    return threads;
}

void ice9::on_event(ice9_event_code code) {



}

#include "ice9_internal.h"
#include "ice9_process_monitor.h"
#include "ice9_thread_monitor.h"
#include "ice9_modules_monitor.h"
#include "ice9_message_monitor.h"
#include "ice9_antidebug.h"