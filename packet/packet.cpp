//
// Created by Артур on 02.10.2026.
//

#include "packet.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string getAgentId()
{
#ifdef _WIN32
    char buffer[MAX_COMPUTERNAME_LENGTH + 1]{};
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(buffer, &size))
    {
        return std::string(buffer, size);
    }
#else
    char buffer[256]{};
    if (gethostname(buffer, sizeof(buffer)) == 0)
    {
        return std::string(buffer);
    }
#endif
    return "unknown";
}

std::string formatTime(std::chrono::system_clock::time_point tp)
{
    auto local = std::chrono::current_zone()->to_local(tp);
    return std::format("{:%Y-%m-%d %H:%M:%S}", local);
}

nlohmann::ordered_json buildPacket(const std::string& agentId, const std::deque<Metrics>& messages)
{
    using json = nlohmann::ordered_json;

    json packet;
    packet["agent_id"] = agentId;
    packet["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    json payload = json::array();
    for (const auto& m : messages)
    {
        payload.push_back({
            {"time", formatTime(m.time)},
            {"process_name", m.processName},
            {"window_title", m.windowTitle},
            {"user_active", m.hasActivity}
        });
    }
    packet["payload"] = std::move(payload);
    return packet;
}
