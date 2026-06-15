#pragma once

#include "pch.h"

namespace Framework
{
    inline DirectX::XMFLOAT4 ToFloat4(DirectX::FXMVECTOR value)
    {
        DirectX::XMFLOAT4 result;
        DirectX::XMStoreFloat4(&result, value);
        return result;
    }

    inline UINT ArgbToAbgr(UINT argb)
    {
        BYTE a = static_cast<BYTE>((argb >> 24) & 0xff);
        BYTE r = static_cast<BYTE>((argb >> 16) & 0xff);
        BYTE g = static_cast<BYTE>((argb >> 8) & 0xff);
        BYTE b = static_cast<BYTE>(argb & 0xff);

        return (a << 24) | (b << 16) | (g << 8) | r;
    }
}
