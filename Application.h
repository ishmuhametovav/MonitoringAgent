//
// Created by Артур on 02.10.2026.
//

#ifndef MONITORINGAGENT_APPLICATION_H
#define MONITORINGAGENT_APPLICATION_H
#include <atomic>
#include <memory>

#include "./metrics/MetricsProvider.h"
#include "./queue/MessageQueue.h"


class Application
{
    std::unique_ptr<MetricsProvider> provider;
    MessageQueue queue;
    std::string serverUrl;
    std::string agentId;
    std::atomic<bool> running{true};

public:
    Application(std::unique_ptr<MetricsProvider> provider, MessageQueue queue,
                std::string serverUrl, std::string agentId);

    int run();
    void stop();
};


#endif //MONITORINGAGENT_APPLICATION_H
