//
// Created by Артур on 02.10.2026.
//

#include "MessageQueue.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

MessageQueue::MessageQueue(std::string path, size_t maxSize)
    : path(std::move(path)), maxSize(maxSize)
{
}

void MessageQueue::push(Metrics message)
{
    if (messages.size() >= maxSize)
    {
        messages.pop_front();
    }
    messages.push_back(std::move(message));
}

size_t MessageQueue::size() const
{
    return messages.size();
}

bool MessageQueue::empty() const
{
    return messages.empty();
}

const std::deque<Metrics>& MessageQueue::getMessages() const
{
    return messages;
}

void MessageQueue::clear()
{
    messages.clear();
}

void MessageQueue::saveToDisk() const
{
    nlohmann::json j = messages;

    std::string tmpPath = path + ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            std::cerr << "Cannot open " << tmpPath << " for writing" << std::endl;
            return;
        }
        out << j.dump(2);
        out.flush();
        if (!out)
        {
            std::cerr << "Write to " << tmpPath << " failed" << std::endl;
            return;
        }
    }

    std::error_code ec;
    std::filesystem::rename(tmpPath, path, ec);
    if (ec)
    {
        std::cerr << "Cannot rename " << tmpPath << " to " << path
            << ": " << ec.message() << std::endl;
        std::filesystem::remove(tmpPath, ec);
    }
}

void MessageQueue::loadFromDisk()
{
    std::ifstream in(path, std::ios::binary);
    if (!in) return;

    try
    {
        nlohmann::json j;
        in >> j;
        for (const auto& item : j)
        {
            messages.push_back(item.get<Metrics>());
            if (messages.size() >= maxSize) break;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to parse " << path << ": " << e.what() << std::endl;
    }
}
