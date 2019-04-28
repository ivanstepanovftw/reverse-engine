//
// Created by root on 24.03.19.
//
#pragma once

#include <utility>
#include <ostream>
#include <reverseengine/common.hh>
#include <reverseengine/value.hh>
#include <reverseengine/core.hh>
#include <reverseengine/scanner.hh>
#include "scanner.hh"


NAMESPACE_BEGIN(RE)

using namespace std;
using namespace std::chrono;
// using std::cout, std::endl;
namespace sfs = std::filesystem;
namespace bio = boost::iostreams;

//
// class Offsets {
// public:
//     Offsets() = delete;
//
//     explicit Offsets(size_t level) {
//         allocated = level;
//         size = 0;
//         off = new uintptr_t[allocated];
//     }
//
//     Offsets(const Offsets& rhs) {
//         allocated = rhs.allocated;
//         size = rhs.size;
//         off = (uintptr_t *) realloc(rhs.allocated);
//         memcpy(off, rhs.off, rhs.size);
//     }
//
//     ~Offsets() {
//         delete[] off;
//     }
//
//     void push_back(uintptr_t&& o) {
//         size++;
//         if (size > allocated)
//             o
//     }
//
// public:
//     uintptr_t allocated;
//     uintptr_t size;
//     uintptr_t *off;
// };


class RegionPaths {
public:
    RegionPaths() = default;

    explicit RegionPaths(const sfs::path& file) : file(file) {};

public:
    sfs::path file;
    // std::vector<Offsets> paths;
    std::vector<std::vector<uintptr_t>> paths;
};


class Pointer {
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
    IProcess &handler;
    sfs::path file;
    std::vector<uintptr_t> offsets;

public:
    Pointer(IProcess &handler, const sfs::path &file, const std::vector<uintptr_t> &offsets)
            : handler(handler), file(file), offsets(offsets) {}

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
};


class PointerScanner {
public:
    explicit PointerScanner(IProcessM& proc)
            : proc(proc) {}

    ~PointerScanner() {}


    std::vector<RegionPaths>
    scan_regions(uintptr_t address) {
        using namespace std;

        std::vector<RegionPaths> result;
        StaticRegions sr(proc);
        RE::Scanner scanner(proc);
        scanner.step = align;

        RE::Cuservalue cuservalue[2];
        cuservalue[0].flags = flag_t::flag_u64;
        const Edata_type dt = Edata_type::INTEGER64;
        const Ematch_type mt = Ematch_type::MATCHEQUALTO;

        paths_resolved = 0;

        RegionStatic *rs_0 = sr.get_region_of_address(address);
        if UNLIKELY(rs_0 != nullptr)
            cout<<"found: "<<rs_0->region.file.filename()<<"+"<<HEX(address - rs_0->address)<<endl;

        for(uintptr_t of_0 = address - max_offset; of_0 <= address; of_0 += align) {
            cuservalue[0].u64 = of_0;
            RE::ByteMatches m_0;
            scanner.scan_regions(m_0, dt, cuservalue, mt);
            for(value_t value_1 : m_0) {
                RegionStatic *rs_1 = sr.get_region_of_address(value_1.address);
                if UNLIKELY(rs_1 != nullptr)
                    cout << "found: "<<rs_1->region.file.filename()<<"+"<<HEX(value_1.address - rs_1->address)
                         << "+"<<HEX(address - rs_0->address)<<endl;

                for(uintptr_t of_1 = value_1.address - max_offset; of_1 <= value_1.address; of_1 += align) {
                    cuservalue[0].u64 = of_1;
                    RE::ByteMatches m_1;
                    scanner.scan_regions(m_1, dt, cuservalue, mt);
                    for(value_t value_2 : m_1) {
                        RegionStatic *rs_2 = sr.get_region_of_address(value_2.address);
                        if UNLIKELY(rs_2 != nullptr) {
                            cout << "found: "<<rs_2->region.file.filename()<<"+"<<HEX(value_2.address - rs_2->address)
                                 << "+"<<HEX(value_1.address - of_1)
                                 << "+"<<HEX(address - rs_0->address)<<endl;
                        }

                        // for(uintptr_t of_2 = value_2.address - max_offset; of_2 < value_2.address; of_2 += align) {
                        //     cuservalue[0].u64 = of_2;
                        //     RE::ByteMatches m_2;
                        //     scanner.scan_regions(m_2, dt, cuservalue, mt);
                        //     for(value_t value_3 : m_2) {
                        //         RegionStatic *rs_3 = sr.get_region_of_address(value_3.address);
                        //         if UNLIKELY(rs_3 != nullptr)
                        //             cout << "found: "<<rs_3->region.file.filename()<<"+"<<HEX(value_3.address - rs_3->address)
                        //                  << "+"<<HEX(value_2.address - rs_2->address)
                        //                  << "+"<<HEX(value_1.address - rs_1->address)
                        //                  << "+"<<HEX(address - rs_0->address)<<endl;
                        //
                        //         // for(uintptr_t of_3 = value_3.address - max_offset; of_3 < value_3.address; of_3 += align) {
                        //         //     cuservalue[0].u64 = of_3;
                        //         //     RE::ByteMatches m_3;
                        //         //     scanner.scan_regions(m_3, dt, cuservalue, mt);
                        //         //     for(value_t value_4 : m_3) {
                        //         //         RegionStatic *rs_4 = sr.get_region_of_address(value_4.address);
                        //         //         if UNLIKELY(rs_4 != nullptr)
                        //         //             cout << "found: "<<rs_4->region.file.filename()<<"+"<<HEX(value_4.address - rs_4->address)
                        //         //                  << "+"<<HEX(value_3.address - rs_3->address)
                        //         //                  << "+"<<HEX(value_2.address - rs_2->address)
                        //         //                  << "+"<<HEX(value_1.address - rs_1->address)
                        //         //                  << "+"<<HEX(address - rs_0->address)<<endl;
                        //         //
                        //         //         // for(uintptr_t of_4 = value_4.address - max_offset; of_4 < value_4.address; of_4 += align) {
                        //         //         //     cuservalue[0].u64 = of_4;
                        //         //         //     RE::ByteMatches m_4;
                        //         //         //     scanner.scan_regions(m_4, dt, cuservalue, mt);
                        //         //         //     for(value_t value_5 : m_4) {
                        //         //         //         RegionStatic *rs_5 = sr.get_region_of_address(value_5.address);
                        //         //         //         if UNLIKELY(rs_5 != nullptr)
                        //         //         //             cout << "found: "<<rs_5->region.file.filename()<<"+"<<HEX(value_5.address - rs_5->address)
                        //         //         //                  << "+"<<HEX(value_4.address - rs_4->address)
                        //         //         //                  << "+"<<HEX(value_3.address - rs_3->address)
                        //         //         //                  << "+"<<HEX(value_2.address - rs_2->address)
                        //         //         //                  << "+"<<HEX(value_1.address - rs_1->address)
                        //         //         //                  << "+"<<HEX(address - rs_0->address)<<endl;
                        //         //         //         //...
                        //         //         //     }
                        //         //         // }
                        //         //     }
                        //         // }
                        //     }
                        // }
                    }
                }
            }
        }

        cout<<"iterations have been done: "<<paths_resolved<<endl;
        return result;
    }

public:
    /// Create file for storing matches
    RE::IProcessM& proc;
    uintptr_t max_level = 4;
    uintptr_t min_offset = 0;
    uintptr_t max_offset = 2048;
    // uintptr_t max_offset = 0x14;
    uintptr_t align = sizeof(u32);
    // uintptr_t align = 1;
    volatile double scan_progress = 0.0;
    volatile bool stop_flag = false;
    uintptr_t paths_resolved = 0;
};

NAMESPACE_END(RE)
