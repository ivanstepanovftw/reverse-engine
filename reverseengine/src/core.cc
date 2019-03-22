#include <reverseengine/core.hh>

RE::phandler_memory::phandler_memory(const RE::phandler_file& rhs) {
    regions_on_map.clear();
    for (const RE::Cregion& region : rhs.regions) {
        char* map = new char[region.size];
        regions_on_map.emplace_back(map);
        ssize_t copied = rhs.read(region.address, map, region.size);
    }
}
