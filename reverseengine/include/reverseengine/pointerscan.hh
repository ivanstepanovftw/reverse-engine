#include <utility>

//
// Created by root on 24.03.19.
//

#pragma once

#include <reverseengine/common.hh>
#include <reverseengine/value.hh>
#include <reverseengine/core.hh>
#include <ostream>


NAMESPACE_BEGIN(RE)

namespace sfs = std::filesystem;
namespace bio = boost::iostreams;


class pointer_swath {
public:
    pointer_swath() = default;
    explicit pointer_swath(const sfs::path& file) : file(file) {};

public:
    sfs::path file;
    std::vector<std::vector<uintptr_t>> offsets;
};


class pointer {
public:
    /*
     * ["PwnAdventure3-Linux-Shipping"+0x034158D8] -> 0x0E9AC000
     * [0x0E9AC000+0x5F0] -> 0x0CFBE680
     * [0x0CFBE680+0x8]   -> 0x1CA66200
     * [0x1CA66200+0x250] -> 0x1EDF34A8
     * [0x1EDF34A8+0x130] -> 0x2ABBE4F0
     * 0x2ABBE4F0+0xC8    == 0x2ABBE680
     * read<float>(0x2ABBE680, &player[0].pos.z);
     */
    /*
     * [r.begin     + offset[0]] -> resolved[0]
     * [resolved[0] + offset[1]] -> resolved[1]
     * [resolved[1] + offset[2]] -> resolved[2]
     * [resolved[2] + offset[3]] -> resolved[3]
     * [resolved[3] + offset[4]] -> resolved[4]
     * resolved[4] + offset[5] == resolved[5] == resolved.back()
     * read<float>(resolved.back(), &player[0].pos.z);
     */
    sfs::path file;
    std::vector<uintptr_t> offsets;

public:
    pointer(IProcess& handler, const sfs::path& file, const std::vector<uintptr_t>& offsets)
    : handler(handler), file(file), offsets(offsets) { }

    std::vector<uintptr_t>
    resolve() {
        std::vector<uintptr_t> ptr_resolved;
        ptr_resolved.reserve(offsets.size());

        uintptr_t last = handler.get_region_by_name(file.string())->address + offsets[0];
        for (size_t i = 1; i < offsets.size(); i++) {
            if (!handler.read(last, &last)) {
                ptr_resolved.resize(i);
                return ptr_resolved;
            }
            ptr_resolved.emplace_back(last);
            last += offsets[i];
        }
        ptr_resolved.emplace_back(last);
        return ptr_resolved;
    }


private:
    IProcess &handler;
};





NAMESPACE_END(RE)
