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
        nil::service::MessagingService* service;
        std::filesystem::path cache_location;
        nil::xalt::transparent_umap<std::filesystem::path> frame_groups;
        nil::xalt::transparent_umap<unique::Frame> unique_frames;
        nil::xalt::transparent_umap<tagged::Frame> tagged_frames;

        // This mutex is to be used to protect the subscribers inside each frames.
        // Currently, there are two locations/threads where the subscribers are accessed:
        // 1. service thread when messages are being handled.
        // 2. post when user wants to mutate the value of a Value object.
        // This mutex is going to be removed only if nil::service allows dispatching callbacks to
        // the main thread which cause other complications like copying of data multiple times
        std::mutex mutex;
    };
}
