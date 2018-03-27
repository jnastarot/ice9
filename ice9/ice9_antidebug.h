#pragma once


void ice9::antidebug_on_thread() {
    bool set_hide = true;
    SetThreadInformation(GetCurrentThread(), (THREAD_INFORMATION_CLASS)ThreadHideFromDebugger, &set_hide, 1);
}