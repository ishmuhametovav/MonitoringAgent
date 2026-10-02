//
// Created by Артур on 02.10.2026.
//

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>

static std::string formatLocalTime(std::time_t t)
{
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    return buffer;
}

void printPayload(const nlohmann::json& packet)
{
    auto now = std::time(nullptr);
    std::cout << "[" << formatLocalTime(now) << "] "
        << "agent_id=" << packet.value("agent_id", "?")
        << ", timestamp=" << packet.value("timestamp", 0)
        << ", records=" << packet.value("payload", nlohmann::json::array()).size()
        << std::endl;

    for (const auto& record : packet.value("payload", nlohmann::json::array()))
    {
        std::cout << "  "
            << record.value("time", "?") << "  "
            << record.value("process_name", "?") << "  "
            << (record.value("user_active", false) ? "[active]  " : "[idle]    ")
            << record.value("window_title", "")
            << std::endl;
    }
    std::cout << std::endl;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    httplib::Server server;

    server.Post("/", [](const httplib::Request& req, httplib::Response& res)
    {
        try
        {
            auto packet = nlohmann::json::parse(req.body);
            printPayload(packet);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to parse body: " << e.what() << std::endl;
            std::cerr << "Raw body: " << req.body.substr(0, 200) << std::endl;
        }

        res.status = 200;
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    std::cout << "server listening on http://localhost:8080" << std::endl;
    server.listen("0.0.0.0", 8080);
    return 0;
}
