#include <iostream>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <boost/iostreams/device/mapped_file.hpp>
#include <bitmask/bitmask.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/random.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/serialization/vector.hpp>
#include <backward/strstream>


#if !defined(NAMESPACE_BEGIN)
#  define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END)
#  define NAMESPACE_END(name) }
#endif


NAMESPACE_BEGIN(RE)

class region {
public:
    size_t begin;
    size_t end;

    region() = default;

    region(size_t a, size_t b)
    : begin(a), end(b) {}

    size_t size() {
        return end-begin;
    }

    friend std::ostream& operator<<(std::ostream& os, const region& region) {
        os << "{begin: " << region.begin << ", end: " << region.end<<"}";
        return os;
    }

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & begin;
        ar & end;
    }
};

class handler {
public:
    std::vector<region> regions;
    pid_t pid{};

    handler() = default;

    handler(pid_t a) {
        pid = a;
        int c = 0;
        int sz;
        for(int i = 0; i<rand()%4+1; i++) {
            sz = rand()%10 * 10;
            regions.emplace_back(c, c+sz);
            c += sz;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const handler& handler) {
        os << "{pid: " << handler.pid << ", regions: {";
        for(const auto &r : handler.regions)
            os<<r<<", ";
        os<<"\b\b}}";
        return os;
    }

private:
    friend class boost::serialization::access;

    template<class Archive = boost::archive::binary_oarchive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & pid;
        ar & regions;
        //std::cout<<"typeid(T).name(): "<<typeid(Archive).name()<<std::endl;
    }
};


NAMESPACE_END(RE)



int main(int argc, char* argv[]) {
    using namespace boost::interprocess;
    using std::cout, std::endl;
    namespace bio = boost::iostreams;
    namespace bi = boost::interprocess;
    srand(static_cast<unsigned int>(time(nullptr)));

    {
        static char const* const SHM_NAME = "sotest-5b4f4154-0c7a-48f4-9be6-33b99094cea4";

        /* write */
        {
            RE::handler h(146);
            //RE::region h (1, 2);
            cout<<"h: "<<h<<", sizeof: "<<sizeof(h)<<endl;

            bi::shared_memory_object shm(bi::open_or_create, SHM_NAME, bi::read_write);
            shm.truncate(sizeof(size_t)*30);
            bi::mapped_region reg(shm, bi::read_write);
            memset(reg.get_address(), 0xFF, sizeof(size_t)*30);

            //std::ostream
            //std::streambuf

            std::ostrstream s(static_cast<char *>(reg.get_address()), reg.get_size());
            boost::archive::binary_oarchive arch(s, boost::archive::no_header);
            arch << h;
        }

        /* read */
        {
            bi::shared_memory_object shm(bi::open_only, SHM_NAME, bi::read_only);
            bi::mapped_region reg(shm, bi::read_only, 0);

            std::istrstream s(static_cast<char *>(reg.get_address()), reg.get_size());
            boost::archive::binary_iarchive arch(s, boost::archive::no_header);
            RE::handler h;
            //RE::region h;
            arch >> h;
            cout<<"h: "<<h<<endl;
        }
    }
    return 0;
}
