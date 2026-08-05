#include <msvc_details.h>
#ifdef _MSVC_LANG

std::wstring cmd::string_to_wstring(const std::string& str)
{
	if (str.empty()) return {};

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
	return wstr;
}

void cmd::start_clangpp(std::string command)
{
	std::wstring cm = string_to_wstring(command);
#ifdef DEBUG
	std::wcout << "Full command line: " << cm << std::endl;
#endif
	STARTUPINFOW si{};
	PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);
	std::wcout << L"Startup clang++\n";
	if (CreateProcessW(nullptr,
		cm.data(),
		nullptr, nullptr, FALSE, 0, nullptr,
		nullptr, &si, &pi)) {

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		std::wcout << L"> clang++ - ul: Compilation exited with code: " << exitCode << L"\n";

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		DWORD err = GetLastError();
		std::wcerr << L"> clang++ - ul: Compilation failed, error: " << err << L"\n";
	}
}
#endif

void cmd::start_clang(std::string command)
{
	std::wstring cm = string_to_wstring(command);
#ifdef DEBUG
	std::wcout << "Full command line: " << cm << std::endl;
#endif
	std::wcout << L"Startup clang\n";
	STARTUPINFOW si{};
	PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);

	if (CreateProcessW(nullptr,
		cm.data(),
		nullptr, nullptr, FALSE, 0, nullptr,
		nullptr, &si, &pi)) {

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		std::wcout << L"> clang - ul: Compilation exited with code: " << exitCode << L"\n";

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		DWORD err = GetLastError();
		std::wcerr << L"> clang - ul: Compilation failed, error: " << err << L"\n";
	}
}