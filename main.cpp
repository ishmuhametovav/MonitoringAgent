#include <chrono>
#include <csignal>

#include "Application.h"
#include "httplib.h"
#include "packet/packet.h"

static Application* globalApp = nullptr;

static void handleSignal(int)
{
    if (globalApp)
    {
        globalApp->stop();
    }
}

int main()
{
    auto provider = createMetricsProvider();
    if (!provider)
    {
        return 1;
    }

    MessageQueue queue("queue.json");

    Application app(std::move(provider),
                    std::move(queue),
                    "http://localhost:8080",
                    getAgentId());

    globalApp = &app;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    return app.run();
}
