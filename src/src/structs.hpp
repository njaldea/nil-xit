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
#include <variant>

namespace nil::xit
{
    struct Core
    {
        nil::service::MessagingService* service;
        std::filesystem::path cache_location;
        std::optional<std::filesystem::path> directory;
        std::unordered_map<std::string, std::variant<unique::Frame, tagged::Frame>> frames;
        // this mutex is to safe guard subscriber tracking
        // this is going to be "solved" if nil/service allows dispatching of callbacks to the
        // messaging thread if post is ran only in messaging thread, this is going to be threadsafe
        // by default
        std::mutex mutex;
    };
}
