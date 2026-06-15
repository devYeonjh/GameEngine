#pragma once

#include <filesystem>
#include <string>

namespace Framework
{
    inline std::wstring GetShaderPathString(const char* sourceFile, const wchar_t* fileName)
    {
        return (std::filesystem::path(sourceFile).parent_path() / L"Shader" / fileName).wstring();
    }

    inline std::filesystem::path GetModelPath(const char* SourceFile, const wchar_t* FileName)
    {
        return std::filesystem::path(SourceFile).parent_path() / L"Models" / FileName;
    }
}
