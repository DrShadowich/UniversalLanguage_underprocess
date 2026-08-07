#pragma once
#include <array>
#include <ulstring.h>
#include <iostream>
#ifdef _MSVC_LANG
#include <Windows.h>
namespace cmd
{
    std::wstring string_to_wstring(const ul::utils::classes::stringi8& str);

    void start_clangpp(ul::utils::classes::stringi8 command);
    void start_clang(ul::utils::classes::stringi8 command);
#else
void start_clang(std::array<classes::stringi8, 2>);
#endif
}