//
// Created by Артур on 02.10.2026.
//

#ifndef MONITORINGAGENT_WINDOWSMETRICSPROVIDER_H
#define MONITORINGAGENT_WINDOWSMETRICSPROVIDER_H

#include "MetricsProvider.h"

class WindowsMetricsProvider : public MetricsProvider
{
    [[nodiscard]]
    bool checkActivity() const;

    [[nodiscard]]
    std::pair<std::string, std::string> getProcessInfo() const;

public:
    [[nodiscard]]
    Metrics getInfo() const override;
};


#endif //MONITORINGAGENT_WINDOWSMETRICSPROVIDER_H
