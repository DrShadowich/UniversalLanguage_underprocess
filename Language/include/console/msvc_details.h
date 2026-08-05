#pragma once
#include <array>
#include <string>
#include <iostream>
#ifdef _MSVC_LANG
#include <Windows.h>
namespace cmd
{
    std::wstring string_to_wstring(const std::string& str);

    void start_clangpp(std::string command);
    void start_clang(std::string command);
#else
void start_clang(std::array<std::string, 2>);
#endif
}