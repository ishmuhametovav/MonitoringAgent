//
// Created by Артур on 02.10.2026.
//

#ifndef MONITORINGAGENT_PACKET_H
#define MONITORINGAGENT_PACKET_H

#include <chrono>
#include <deque>
#include <string>
#include "nlohmann/json_fwd.hpp"
#include "../metrics/MetricsProvider.h"

std::string getAgentId();
std::string formatTime(std::chrono::system_clock::time_point tp);
nlohmann::ordered_json buildPacket(const std::string& agentId, const std::deque<Metrics>& messages);

#endif //MONITORINGAGENT_PACKET_H
