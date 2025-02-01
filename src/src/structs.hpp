#pragma once

#include <nil/xit/structs.hpp>

#include "tagged/structs.hpp"
#include "unique/structs.hpp"

#include <nil/service/structs.hpp>

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace nil::xit
{
    struct Core
    {
        nil::service::MessagingService* service;
        std::filesystem::path cache_location;
        std::optional<std::filesystem::path> directory;
        std::unordered_map<std::string, unique::Frame> unique_frames;
        std::unordered_map<std::string, tagged::Frame> tagged_frames;

        // This mutex is to be used to protect the subscribers inside each frames.
        // Currently, there are two locations/threads where the subscribers are accessed:
        // 1. service thread when messages are being handled.
        // 2. post when user wants to mutate the value of a Value object.
        // This mutex is going to be removed only if nil::service allows dispatching callbacks to
        // the main thread which cause other complications like copying of data multiple times
        std::mutex mutex;
    };
}
