#pragma once

#include <nil/xit/structs.hpp>

#include "tagged/structs.hpp"
#include "unique/structs.hpp"

#include <nil/service/structs.hpp>
#include <nil/xalt/transparent_stl.hpp>

#include <filesystem>
#include <mutex>

namespace nil::xit
{
    struct Core
    {
        nil::service::IRunnableService* run_service;
        nil::service::IMessagingService* msg_service;
        std::filesystem::path cache_location;
        nil::xalt::transparent_umap<std::filesystem::path> groups;
        nil::xalt::transparent_umap<unique::Frame> unique_frames;
        nil::xalt::transparent_umap<tagged::Frame> tagged_frames;
    };
}
