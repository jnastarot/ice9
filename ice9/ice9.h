#pragma once


#define ICE9_DEBUG

#define ICE9_THREAD_ENABLED
#define ICE9_WINDOW_HOOK_ENABLED
#define ICE9_WINDOW_RAW_HOOK_ENABLED
#define ICE9_WINDOW_HOOK_EXPERIMENTAL_ENABLED
#define ICE9_ANTI_DEBUG_ENABLED
#define ICE9_PREVENT_WINDOW_HOOK_DLL_ENABLED
#define ICE9_DLL_MONITOR_ENABLED

enum ice9_event_code {
    ice9_ok,
    ice9_thread_remote,
    ice9_thread_unbase,
    ice9_thread_suspected_address,
    ice9_thread_detach_isnt_pool,

    ice9_bad_dll_loaded,

    ice9_message_emulated,
    ice9_input_emulated,
    ice9_window_not_found,
};

struct msg_manager {
    HWND    hwnd;
    WNDPROC proc;
    HHOOK   hook;
    
    struct {
        int up;
        int down;
        unsigned int blocked;
    }key_press[256];

    BYTE mouse_press[3];
};


class ice9{
    HMODULE main_module;
    std::vector<BYTE *> funcs;
    std::vector<BYTE *> suspected_funcs;
    std::vector<unsigned int> threads;
    std::vector<msg_manager>  windows;

    BOOL is_64 = false;
    
    BYTE obj_type_thread = 0xFF;
    BYTE obj_type_process = 0xFF;

    void * vtbl_pDispatchA = 0;
    void * vtbl_pDispatchW = 0;
    BYTE * o_DispatchClientMessage_A = 0;
    BYTE * o_DispatchClientMessage_W = 0;

    BYTE * dll_notify_cookie = 0;
    BYTE * o_ClientLoadLibrary = 0;

    PVOID ice9::get_thread_start_address(HANDLE hthread);
    bool  ice9::is_remote_thread(DWORD id);
    ice9_event_code ice9::check_current_thread();

    void ice9::set_window_hook(msg_manager& mgr);   
    void ice9::unset_window_hook(msg_manager& mgr);

    void ice9::unset_window_hook_exp();
    void ice9::set_window_hook_exp();

    void ice9::set_raw_window_hook(msg_manager& mgr);
    void ice9::unset_raw_window_hook(msg_manager& mgr);

    void ice9::set_dll_hook();
    void ice9::unset_dll_hook();

    void ice9::antidebug_on_thread();
public:
    ice9::ice9();
    ice9::~ice9();

    void ice9::do_init_ice9();

    void ice9::on_event(ice9_event_code code);

    void ice9::on_thread_attach(HMODULE hmodule, DWORD reason, LPVOID reserved);
    void ice9::on_thread_detach(HMODULE hmodule, DWORD reason, LPVOID reserved);

    void ice9::on_window_attach(HWND hwnd);
    void ice9::on_window_detach(HWND hwnd);

    void ice9::on_dll_attach(std::wstring dll_path);
    void ice9::on_dll_detach(std::wstring dll_path);
public:
    ice9& ice9::instance();

    BYTE * ice9::get_dispatch_client_message_a();
    BYTE * ice9::get_dispatch_client_message_w();
    BYTE * ice9::get_client_load_library();

    std::vector<msg_manager>& ice9::get_windows();
    std::vector<unsigned int>& ice9::get_threads();
};

