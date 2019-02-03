#include <iostream>
#include <fstream>
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
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <strstream>


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

    size_t size() const {
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



//class counter_streambuf : public std::streambuf
//{
//public:
//    using std::streambuf::streambuf;
//
//    std::streamsize
//    sputn(const char_type* __s, std::streamsize __n)
//    { this->m_size += __n; return __n; }
//
//    size_t size() { return m_size; }
//
//private:
//    size_t m_size = 0;
//};


class counter_streambuf : public std::streambuf
{
public:
    size_t size() const { return m_size; }

protected:
    std::streamsize xsputn(const char_type* __s, std::streamsize __n) override {
        this->m_size += __n;
        return __n;
    }

private:
    size_t m_size = 0;
};

NAMESPACE_END(RE)


int main(int argc, char* argv[]) {
    using std::cout, std::endl;
    namespace bio = boost::iostreams;
    namespace bi = boost::interprocess;
    //srand(static_cast<unsigned int>(time(nullptr)));

    {
        std::string path = "pornhub";

        /* write */
        {
            std::string magic_bytes = "RE::handler_mmap";
            uint8_t a = 0xFF;
            RE::handler h(146);

            size_t total_scan_bytes = 0;
            for (const RE::region& r : h.regions) {
                total_scan_bytes += r.size();
            }

            RE::counter_streambuf csb;
            boost::archive::binary_oarchive oa(csb, boost::archive::no_header);
            oa << magic_bytes;
            oa << a;
            oa << h;

            bio::mapped_file snapshot_mf;
            bio::mapped_file_params snapshot_mf_;

            snapshot_mf_.path = path;
            snapshot_mf_.flags = bio::mapped_file::mapmode::readwrite;
            snapshot_mf_.new_file_size = total_scan_bytes + csb.size();
            snapshot_mf.open(snapshot_mf_);
            if (!snapshot_mf.is_open())
                throw std::invalid_argument("can not open '" + path + "'");

            char *snapshot = snapshot_mf.data();

            for (size_t i = 0; i < total_scan_bytes + csb.size(); i++)
                memset(snapshot + i, 0xff, 1);

            std::ostrstream s(snapshot, csb.size());
            boost::archive::binary_oarchive arch(s, boost::archive::no_header);
            arch << magic_bytes;
            arch << a;
            arch << h;
            s.flush();

            snapshot += csb.size();

            for (const RE::region& r : h.regions) {
                for (size_t i = 0; i < r.size(); i++)
                    memset(snapshot++, 0x33, 1);
            }
        }

        /*read*/
        {

        }


        /* write */
        //{
        //    RE::handler h(146);
        //    //RE::region h (1, 2);
        //    cout<<"h: "<<h<<", sizeof: "<<sizeof(h)<<endl;
        //
        //    bi::shared_memory_object shm(bi::open_or_create, SHM_NAME, bi::read_write);
        //    shm.truncate(sizeof(size_t)*30);
        //    bi::mapped_region reg(shm, bi::read_write);
        //    memset(reg.get_address(), 0xFF, sizeof(size_t)*30);
        //
        //    //std::ostream
        //    //std::streambuf
        //
        //    std::ostrstream s(static_cast<char *>(reg.get_address()), reg.get_size());
        //    boost::archive::binary_oarchive arch(s, boost::archive::no_header);
        //    arch << h;
        //}
        //
        ///* read */
        //{
        //    bi::shared_memory_object shm(bi::open_only, SHM_NAME, bi::read_only);
        //    bi::mapped_region reg(shm, bi::read_only, 0);
        //
        //    std::istrstream s(static_cast<char *>(reg.get_address()), reg.get_size());
        //    boost::archive::binary_iarchive arch(s, boost::archive::no_header);
        //    RE::handler h;
        //    //RE::region h;
        //    arch >> h;
        //    cout<<"h: "<<h<<endl;
        //}
    }
    return 0;
}
