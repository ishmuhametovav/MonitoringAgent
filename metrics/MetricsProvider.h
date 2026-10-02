//
// Created by Артур on 02.10.2026.
//

#ifndef MONITORINGAGENT_METRICSPROVIDER_H
#define MONITORINGAGENT_METRICSPROVIDER_H
#include <chrono>
#include <memory>
#include <string>

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

struct Metrics
{
    std::chrono::system_clock::time_point time;
    std::string processName;
    std::string windowTitle;
    bool hasActivity;
};

inline void to_json(nlohmann::json& j, const Metrics& metrics)
{
    j["time"] = std::chrono::duration_cast<std::chrono::seconds>(
        metrics.time.time_since_epoch()).count();
    j["process_name"] = metrics.processName;
    j["window_title"] = metrics.windowTitle;
    j["user_active"] = metrics.hasActivity;
}

inline void from_json(const nlohmann::json& j, Metrics& metrics)
{
    auto seconds = j.at("time").get<int64_t>();
    metrics.time = std::chrono::system_clock::time_point{std::chrono::seconds{seconds}};
    j.at("process_name").get_to(metrics.processName);
    j.at("window_title").get_to(metrics.windowTitle);
    j.at("user_active").get_to(metrics.hasActivity);
}

constexpr int INTERVAL = 5000;

class MetricsProvider
{
public:
    virtual ~MetricsProvider() = default;

    [[nodiscard]]
    virtual Metrics getInfo() const = 0;
};

std::unique_ptr<MetricsProvider> createMetricsProvider();

#endif //MONITORINGAGENT_METRICSPROVIDER_H
