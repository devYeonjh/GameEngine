#pragma once

namespace Framework::Debug {

    /**
     *  OutputDebugString를 사용하기 위한 wrapper 함수
     */
    inline void DebugLog(const wchar_t* format, ...) {
        wchar_t buffer[256];
        va_list args;
        va_start(args, format);
        vswprintf_s(buffer, format, args);
        va_end(args);
        OutputDebugString(buffer);
    }

}
