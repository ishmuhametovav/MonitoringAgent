//
// Created by Артур on 02.10.2026.
//

#ifndef MONITORINGAGENT_MESSAGEQUEUE_H
#define MONITORINGAGENT_MESSAGEQUEUE_H

#include <deque>
#include "../metrics/MetricsProvider.h"

constexpr size_t DEFAULT_MAX_SIZE = 100;

class MessageQueue
{
    std::deque<Metrics> messages;
    std::string path;
    size_t maxSize;

public:
    explicit MessageQueue(std::string path, size_t maxSize = DEFAULT_MAX_SIZE);

    void push(Metrics message);

    [[nodiscard]] size_t size() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::deque<Metrics>& getMessages() const;

    void clear();

    void saveToDisk() const;
    void loadFromDisk();
};


#endif //MONITORINGAGENT_MESSAGEQUEUE_H
