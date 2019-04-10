//
// Created by root on 24.03.19.
//

#pragma once

#include <reverseengine/common.hh>
#include <reverseengine/value.hh>
#include <reverseengine/core.hh>
#include <ostream>

NAMESPACE_BEGIN(RE)


class region_swath {
    std::vector<region> regions;
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
    std::string region_name;
    std::vector<ptrdiff_t> offsets;

public:
    pointer(phandler_i& handler, const std::string& region_name, const std::vector<ptrdiff_t>& offsets)
    : handler(handler), region_name(region_name), offsets(offsets) { }



    std::vector<uintptr_t>
    resolve() {
        std::vector<uintptr_t> ptr_resolved;
        ptr_resolved.reserve(offsets.size());

        uintptr_t last = handler.get_region_by_name(region_name)->address + offsets[0];
        for (size_t i = 1; i < offsets.size(); i++) {
            if (!handler.read(last, &last)) {
                //TODO[CRITICAL]: ambiguous what to return? throw something?
                std::cerr<<"resolve: Cannot resolve pointer: offset "<<i<<std::endl;
                return ptr_resolved;
            }
            ptr_resolved.emplace_back(last);
            last += offsets[i];
        }
        ptr_resolved.emplace_back(last);
        return ptr_resolved;
    }

    std::string
    str(const std::vector<uintptr_t>& resolved) const {
        std::ostringstream oss;
        oss<<"[\""<<region_name<<"\"+"<<HEX(offsets[0])<<"] -> "<<HEX(resolved[0])<<"\n";
        for (size_t i = 0; i < offsets.size()-2; i++) {
            oss<<"["<<HEX(resolved[i])<<"+"<<HEX(offsets[i+1])<<"] -> "<<HEX(resolved[i+1])<<"\n";
        }
        oss<<""<<HEX(resolved.back())<<"+"<<HEX(offsets.back())<<" = "<<HEX(resolved.back()+offsets.back())<<"\n";
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const pointer& pointer) {
        //TODO[CRITICAL]: ambiguous
        std::vector<uintptr_t> resolved = {0,0,0,0,0,0,0,0,0,0,0};
        os << pointer.str(resolved);
        return os;
    }

private:
    phandler_i &handler;
};





NAMESPACE_END(RE)
