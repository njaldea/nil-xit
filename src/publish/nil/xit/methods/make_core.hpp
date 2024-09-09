#pragma once

#include "../structs.hpp"

#include <nil/service/IService.hpp>

#include <memory>

namespace nil::xit
{
    std::unique_ptr<Core, void (*)(Core*)> make_core(nil::service::IService& service);
}
