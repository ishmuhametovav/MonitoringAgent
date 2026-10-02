//
// Created by Артур on 02.10.2026.
//

#include "X11MetricsProvider.h"

#include <X11/Xatom.h>

#include <cstdio>
#include <fstream>
#include <string>

#include <limits.h>
#include <unistd.h>

X11MetricsProvider::X11MetricsProvider()
{
    display = XOpenDisplay(nullptr);
    if (!display) return;

    root = DefaultRootWindow(display);

    activeWindowAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    wmPidAtom = XInternAtom(display, "_NET_WM_PID", False);
    netWmNameAtom = XInternAtom(display, "_NET_WM_NAME", False);
    utf8StringAtom = XInternAtom(display, "UTF8_STRING", False);

    int eventBase = 0;
    int errorBase = 0;
    if (XScreenSaverQueryExtension(display, &eventBase, &errorBase))
    {
        screenSaverInfo = XScreenSaverAllocInfo();
    }
}

X11MetricsProvider::~X11MetricsProvider()
{
    if (screenSaverInfo)
    {
        XFree(screenSaverInfo);
        screenSaverInfo = nullptr;
    }
    if (display)
    {
        XCloseDisplay(display);
        display = nullptr;
    }
}

bool X11MetricsProvider::readProperty(Window window, Atom property,
                                      Atom requiredType,
                                      unsigned char** data,
                                      unsigned long* length,
                                      int* format) const
{
    constexpr long MAX_ITEMS = 1024;

    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long itemsCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* value = nullptr;

    int status = XGetWindowProperty(
        display, window, property,
        0, MAX_ITEMS, False, requiredType,
        &actualType, &actualFormat, &itemsCount, &bytesAfter, &value);

    if (status != Success || !value || actualFormat == 0)
    {
        if (value) XFree(value);
        return false;
    }

    *data = value;
    *length = itemsCount;
    if (format) *format = actualFormat;
    return true;
}

Window X11MetricsProvider::getActiveWindow() const
{
    unsigned char* value = nullptr;
    unsigned long length = 0;

    if (!readProperty(root, activeWindowAtom, XA_WINDOW, &value, &length))
        return 0;

    Window active = *reinterpret_cast<Window*>(value);
    XFree(value);
    return active;
}

Window X11MetricsProvider::findWindowWithPid(Window window) const
{
    while (window != 0)
    {
        unsigned char* value = nullptr;
        unsigned long length = 0;

        if (readProperty(window, wmPidAtom, XA_CARDINAL, &value, &length))
        {
            XFree(value);
            return window;
        }

        Window rootWindow = 0;
        Window parent = 0;
        Window* children = nullptr;
        unsigned int childrenCount = 0;

        if (!XQueryTree(display, window, &rootWindow, &parent, &children, &childrenCount))
            return 0;

        if (children) XFree(children);

        if (parent == rootWindow || parent == 0)
            return 0;

        window = parent;
    }
    return 0;
}

int X11MetricsProvider::getWindowPid(Window window) const
{
    Window target = findWindowWithPid(window);
    if (target == 0) return -1;

    unsigned char* value = nullptr;
    unsigned long length = 0;
    int format = 0;

    if (!readProperty(target, wmPidAtom, XA_CARDINAL, &value, &length, &format))
        return -1;

    int pid = -1;
    if (format == 32 && length >= 1)
    {
        pid = static_cast<int>(*reinterpret_cast<unsigned long*>(value));
    }
    XFree(value);
    return pid;
}

std::string X11MetricsProvider::getWindowTitle(Window window) const
{
    unsigned char* value = nullptr;
    unsigned long length = 0;
    int format = 0;

    if (readProperty(window, netWmNameAtom, utf8StringAtom, &value, &length, &format))
    {
        if (format == 8)
        {
            std::string title(reinterpret_cast<char*>(value), length);
            XFree(value);
            return title;
        }
        XFree(value);
    }

    char* name = nullptr;
    if (XFetchName(display, window, &name) && name)
    {
        std::string title(name);
        XFree(name);
        return title;
    }

    return {};
}

std::string X11MetricsProvider::getProcessName(int pid)
{
    if (pid <= 0) return {};

    char linkPath[64];
    std::snprintf(linkPath, sizeof(linkPath), "/proc/%d/exe", pid);

    char buffer[PATH_MAX];
    ssize_t bytesRead = readlink(linkPath, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        std::string path(buffer);
        auto separator = path.find_last_of('/');
        if (separator != std::string::npos)
            return path.substr(separator + 1);
        return path;
    }

    std::snprintf(linkPath, sizeof(linkPath), "/proc/%d/comm", pid);
    std::ifstream stream(linkPath);
    if (!stream)
        return {};

    std::string name;
    std::getline(stream, name);
    return name;
}

bool X11MetricsProvider::checkActivity() const
{
    if (!screenSaverInfo) return false;

    if (!XScreenSaverQueryInfo(display, root, screenSaverInfo))
        return false;

    return screenSaverInfo->idle < INTERVAL;
}

std::pair<std::string, std::string> X11MetricsProvider::getProcessInfo() const
{
    if (!display) return {};

    Window active = getActiveWindow();
    if (active == 0) return {};

    int pid = getWindowPid(active);
    std::string executable = getProcessName(pid);
    std::string windowTitle = getWindowTitle(active);

    return {std::move(executable), std::move(windowTitle)};
}

Metrics X11MetricsProvider::getInfo() const
{
    auto [name, title] = getProcessInfo();
    bool activity = checkActivity();
    return {
        .time = std::chrono::system_clock::now(), .processName = name, .windowTitle = title, .hasActivity = activity
    };
}

std::unique_ptr<MetricsProvider> createMetricsProvider()
{
    return std::make_unique<X11MetricsProvider>();
}
