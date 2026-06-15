#include "message.fbs.h"

#include <flatbuffers/verifier.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>

namespace
{
    std::string read_binary(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::in);
        return {std::istream_iterator<char>(file >> std::noskipws), std::istream_iterator<char>()};
    }

    void print_frame_cache_save(const nil::xit::fbs::FrameCacheSave* save)
    {
        const auto* cache = save->cache();
        std::cout << "FrameCacheSave\n";
        std::cout << "  cache.id: " << cache->id()->string_view() << '\n';
        std::cout << "  cache.group: " << cache->group()->string_view() << '\n';
        std::cout << "  cache.path: " << cache->path()->string_view() << '\n';
        std::cout << "  cache.content_size: " << cache->content()->size() << '\n';

        std::cout << "  files (" << cache->files()->size() << ")\n";
        for (const auto* file_info : *cache->files())
        {
            std::cout << "    - group=" << file_info->group()->string_view();
            std::cout << ", path=" << file_info->path()->string_view();
            std::cout << '\n';
        }

        std::cout << "  groups (" << cache->groups()->size() << ")\n";
        for (const auto* group : *cache->groups())
        {
            std::cout << "    - " << group->string_view() << '\n';
        }

        std::cout << "  options (" << cache->options()->size() << ")\n";
        for (const auto* option : *cache->options())
        {
            std::cout << "    - " << option->key()->string_view();
            std::cout << '=' << option->value()->string_view() << '\n';
        }

        std::cout << "  aliases (" << save->groups()->size() << ")\n";
        for (const auto* alias : *save->groups())
        {
            std::cout << "    - group=" << alias->group()->string_view();
            std::cout << ", path=" << alias->path()->string_view() << '\n';
        }
    }
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: cache-decoder <cache-binary-file>\n";
        return 1;
    }

    const std::filesystem::path input_path = argv[1];
    if (!std::filesystem::exists(input_path))
    {
        std::cerr << "error: file does not exist: " << input_path << '\n';
        return 1;
    }

    const auto content = read_binary(input_path);
    if (content.empty())
    {
        std::cerr << "error: file is empty or unreadable\n";
        return 1;
    }

    flatbuffers::Verifier verifier(
        reinterpret_cast<const std::uint8_t*>(content.data()), // NOLINT
        content.size()
    );
    if (verifier.VerifyBuffer<nil::xit::fbs::FrameCacheSave>())
    {
        const auto* save = flatbuffers::GetRoot<nil::xit::fbs::FrameCacheSave>(content.data());
        print_frame_cache_save(save);
        return 0;
    }

    std::cerr << "error: unsupported binary format; expected FrameCacheSave\n";
    return 2;
}
