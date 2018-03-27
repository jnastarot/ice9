#pragma once



PVOID ice9::get_thread_start_address(HANDLE hthread) {
    NTSTATUS ntStatus;
    PVOID dwStartAddress;

    ntStatus = _NtQueryInformationThread(funcs[0])(
        GetCurrentThread(), (THREADINFOCLASS)ThreadQuerySetWin32StartAddress, &dwStartAddress, sizeof(dwStartAddress), NULL
        );

    if (ntStatus != STATUS_SUCCESS) return 0;
    return dwStartAddress;
}


bool  ice9::is_remote_thread(DWORD id) {
    ULONG retlen = 0;
    ULONG table_length = 0;
    PSYSTEM_HANDLE_INFORMATION p_handle_table = 0;
    bool ret_status = true;

    while (1) {
        NTSTATUS status = _NtQuerySystemInformation(funcs[1])((SYSTEM_INFORMATION_CLASS)SystemHandleInformation,
            p_handle_table, table_length, &retlen
            );

        if (status == STATUS_SUCCESS) {
            break;
        }

        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            table_length += retlen;
            if (p_handle_table) { delete[]p_handle_table; }
            p_handle_table = (PSYSTEM_HANDLE_INFORMATION)new BYTE[table_length];
        }
    }

    if (this->obj_type_thread == 0xFF) {
        POBJECT_TYPE_INFORMATION objInfo = (POBJECT_TYPE_INFORMATION)new BYTE[0x1000];
        for (int i = 0; i < p_handle_table->HandleCount; i++) {
            if (p_handle_table->Handles[i].ProcessId == GetCurrentProcessId()) {
                if (_NtQueryObject(funcs[2])((HANDLE)p_handle_table->Handles[i].Handle, (OBJECT_INFORMATION_CLASS)ObjectTypeInformation, objInfo, 0x1000, NULL) == STATUS_SUCCESS) {
                    if (!lstrcmpW(objInfo->Name.Buffer, L"Thread")) {
                        this->obj_type_thread = p_handle_table->Handles[i].ObjectTypeNumber;
                    }
                }
            }
        }
        delete[]objInfo;
    }

    if (this->obj_type_thread != 0xFF) {
        for (int i = 0; i < p_handle_table->HandleCount; i++) {
            if (p_handle_table->Handles[i].ProcessId == GetCurrentProcessId() && p_handle_table->Handles[i].ObjectTypeNumber == this->obj_type_thread) {
                if (GetThreadId((HANDLE)p_handle_table->Handles[i].Handle) == id) {
                    ret_status = false;
                }
            }
        }
    }
    else {
        ret_status = false;
    }

    delete[]p_handle_table;

    return ret_status;
}


ice9_event_code ice9::check_current_thread() {

    if (!is_remote_thread(GetCurrentThreadId())) {
        PVOID StartAddress = get_thread_start_address(GetCurrentThread());
        if (StartAddress) {

#ifdef _M_IX86
            MEMORY_BASIC_INFORMATION32 MBI;
#else
            MEMORY_BASIC_INFORMATION64 MBI;
#endif
            DWORD mbi_size = sizeof(MBI);

            if (VirtualQuery(StartAddress, (MEMORY_BASIC_INFORMATION*)&MBI, mbi_size)) {

                if (MBI.Type&MEM_IMAGE) { 

                    for (auto& func : suspected_funcs) {
                        if (func == StartAddress) {
                            return ice9_event_code::ice9_thread_suspected_address;
                        }
                    }

                    return ice9_event_code::ice9_ok;
                }
            }
        }
        return ice9_event_code::ice9_thread_unbase;
    }

    return ice9_event_code::ice9_thread_remote;
}


void ice9::on_thread_attach(HMODULE hmodule, DWORD reason, LPVOID reserved) {
#ifdef ICE9_ANTI_DEBUG_ENABLED
    antidebug_on_thread();
#endif

#ifndef ICE9_THREAD_ENABLED
    return;
#endif

    ice9_event_code thread_code = check_current_thread();

    if (thread_code) {
#ifdef ICE9_DEBUG  
        switch (thread_code) {
        case ice9_event_code::ice9_thread_remote: {
            printf("thread [%x] was closed reason : [Remoted thread]\n", GetCurrentThreadId());
            break;
        }
        case ice9_event_code::ice9_thread_unbase: {
            printf("thread [%x] was closed reason : [Base address not IMAGE]\n", GetCurrentThreadId());
            break;
        }
        case ice9_event_code::ice9_thread_suspected_address: {
            printf("thread [%x] was closed reason : [Suspected base address]\n", GetCurrentThreadId());
            break;
        }
        }
#endif
        
        on_event(thread_code);
        TerminateThread(GetCurrentThread(), 0);
    }
    else {
#ifdef ICE9_DEBUG
        printf("attached new thread [%x]\n", GetCurrentThreadId());
#endif
        for (auto& thread_id : threads) {
            if (thread_id == GetCurrentThreadId()) { return; }
        }
        
        threads.push_back(GetCurrentThreadId());
    }
}

void ice9::on_thread_detach(HMODULE hmodule, DWORD reason, LPVOID reserved) {


    for (unsigned int thread_idx = 0; thread_idx < threads.size();thread_idx++) {
        if (thread_idx == GetCurrentThreadId()) {

#ifdef ICE9_DEBUG
            printf("detached thread [%x]\n", GetCurrentThreadId());
#endif
            threads.erase(threads.begin() + thread_idx);
            return;
        }
    }

#ifdef ICE9_DEBUG   
    printf("detached thread [%x] wasnt in thread pool!", GetCurrentThreadId());
#endif

    on_event(ice9_thread_detach_isnt_pool);
}