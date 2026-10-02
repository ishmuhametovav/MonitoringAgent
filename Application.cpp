//
// Created by Артур on 02.10.2026.
//

#include "Application.h"

#include "httplib.h"
#include "packet/packet.h"

Application::Application(std::unique_ptr<MetricsProvider> provider, MessageQueue queue,
                         std::string serverUrl, std::string agentId) : provider(std::move(provider)),
                                                                       queue(std::move(queue)),
                                                                       serverUrl(std::move(serverUrl)),
                                                                       agentId(std::move(agentId))
{
}

void Application::stop()
{
    running = false;
}

int Application::run()
{
    using namespace std::chrono_literals;

    queue.loadFromDisk();

    httplib::Client client(serverUrl);
    client.set_connection_timeout(5, 0);
    client.set_read_timeout(5, 0);

    auto lastCollect = std::chrono::steady_clock::now();
    auto lastSend = lastCollect;

    while (running)
    {
        auto now = std::chrono::steady_clock::now();

        if (now - lastCollect >= 5s)
        {
            lastCollect = now;
            queue.push(provider->getInfo());
        }

        bool timeToSend = (now - lastSend >= 30s);
        bool queueReady = (queue.size() >= 10);

        if ((timeToSend || queueReady) && !queue.empty())
        {
            lastSend = now;

            auto packet = buildPacket(agentId, queue.getMessages());
            auto res = client.Post("/", packet.dump(), "application/json");

            if (res && res->status >= 200 && res->status < 300)
            {
                queue.clear();
            }
            else
            {
                std::cerr << "Send failed, keeping data on disk" << std::endl;
                queue.saveToDisk();
            }
        }

        std::this_thread::sleep_for(200ms);
    }

    queue.saveToDisk();
    return 0;
}
