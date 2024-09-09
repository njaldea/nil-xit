#pragma once

#include <nil/xit/structs.hpp>

#include "Frame.hpp"

#include <nil/service/IService.hpp>

#include <unordered_map>

namespace nil::xit
{
    struct Core
    {
        nil::service::IService* service;
        std::unordered_map<std::string, Frame> frames;
    };
}
