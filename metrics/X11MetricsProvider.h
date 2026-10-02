//
// Created by Артур on 02.10.2026.
//

#ifndef MONITORINGAGENT_X11METRICSPROVIDER_H
#define MONITORINGAGENT_X11METRICSPROVIDER_H

#include "MetricsProvider.h"

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include <string>
#include <utility>

class X11MetricsProvider : public MetricsProvider
{
    Display* display = nullptr;
    Window root = 0;

    Atom activeWindowAtom = 0;
    Atom wmPidAtom = 0;
    Atom netWmNameAtom = 0;
    Atom utf8StringAtom = 0;

    XScreenSaverInfo* screenSaverInfo = nullptr;

    [[nodiscard]]
    bool readProperty(Window window, Atom property, Atom requiredType,
                      unsigned char** data, unsigned long* length,
                      int* format = nullptr) const;

    [[nodiscard]]
    Window getActiveWindow() const;

    [[nodiscard]]
    Window findWindowWithPid(Window window) const;

    [[nodiscard]]
    int getWindowPid(Window window) const;

    [[nodiscard]]
    std::string getWindowTitle(Window window) const;

    [[nodiscard]]
    static std::string getProcessName(int pid);

    [[nodiscard]]
    bool checkActivity() const;

    [[nodiscard]]
    std::pair<std::string, std::string> getProcessInfo() const;

public:
    X11MetricsProvider();
    ~X11MetricsProvider() override;

    [[nodiscard]]
    Metrics getInfo() const override;
};

#endif //MONITORINGAGENT_X11METRICSPROVIDER_H
