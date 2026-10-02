//
// Created by Артур on 02.10.2026.
//

#include "WindowsMetricsProvider.h"
#include <filesystem>
#include <windows.h>

static std::string toUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        wstr.data(), static_cast<int>(wstr.size()),
        nullptr, 0, nullptr, nullptr);

    if (size <= 0) return {};

    std::string result(size, '\0');
    WideCharToMultiByte(
        CP_UTF8, 0,
        wstr.data(), (int)wstr.size(),
        result.data(), size, nullptr, nullptr);
    return result;
}

bool WindowsMetricsProvider::checkActivity() const
{
    LASTINPUTINFO lii{};
    lii.cbSize = sizeof(LASTINPUTINFO);
    if (!GetLastInputInfo(&lii)) return false;
    return (GetTickCount() - lii.dwTime) < INTERVAL;
}

std::pair<std::string, std::string> WindowsMetricsProvider::getProcessInfo() const
{
    constexpr int BUFFER_SIZE = 1024;
    wchar_t windowTitle[BUFFER_SIZE]{0};

    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return {};

    GetWindowTextW(hwnd, windowTitle, BUFFER_SIZE);

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    std::wstring processPath;
    if (processId != 0)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                      FALSE, processId);
        if (hProcess)
        {
            wchar_t buffer[BUFFER_SIZE]{};
            DWORD size = BUFFER_SIZE;
            if (QueryFullProcessImageNameW(hProcess, 0, buffer, &size) && size > 0)
            {
                processPath.assign(buffer, size);
            }
            CloseHandle(hProcess);
        }
    }
    std::string executable;
    if (!processPath.empty())
    {
        std::wstring filename = std::filesystem::path(processPath).filename().wstring();
        executable = toUtf8(filename);
    }

    return {std::move(executable), toUtf8(windowTitle)};
}

Metrics WindowsMetricsProvider::getInfo() const
{
    auto [name, title] = getProcessInfo();
    bool activity = checkActivity();
    return {
        .time = std::chrono::system_clock::now(), .processName = name, .windowTitle = title, .hasActivity = activity
    };
}

std::unique_ptr<MetricsProvider> createMetricsProvider()
{
    return std::make_unique<WindowsMetricsProvider>();
}
