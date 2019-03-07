#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

class some_class_t {
private:
    std::string _str = "hi";
    //std::vector<byte> _very_big;
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & _str;
    }
};

int main(int argc, char* argv[]) {
    namespace bio = boost::iostreams;

    system("rm -f Boost");
    std::string path = "Boost";

    std::ofstream stream(path, std::ios_base::out | std::ios_base::binary);
    boost::archive::binary_oarchive archive(stream, boost::archive::no_header);
    some_class_t some;
    archive << some;
    stream.flush();
    system("echo \\$ xxd Boost && xxd Boost");
    /*! Expected output:
     * 00000000: 3132 3333 3333 0000 0000 0002 0000 0000  123333..........
     * 00000010: 0000 0068 69                             ...hi
     *
     * Got overflow exception, because `archive` inherited size from `mf`:
     *   what():  write area exhausted: iostream error
     */

    bio::mapped_file mf;
    bio::mapped_file_params params;
    params.path = path;
    params.flags = bio::mapped_file::mapmode::readwrite;
    params.new_file_size = 0;
    mf.open(params);
    if (!mf.is_open())
        throw std::invalid_argument("can not open '" + path + "'");

    // And now I wanted to copy memory from another process with "process_vm_readv" to file.
    // But let's assume that it would be "memset" instead of "process_vm_readv", because they are similar.
    // I know size, so...
    size_t s = 12;
    mf.resize(mf.size() + s);
    memset(mf.data() + static_cast<size_t>(stream.tellp()), 0x33, s);
    system("echo \\$ xxd Boost && xxd Boost");
    /*! Expected output:
     * 00000000: 3132 3333 3333 0000 0000 0002 0000 0000  123333.............hi
     * ...
     * 000004e0: 3333 3333 3333 33                        3333333
     */

    mf.close();

    assert(stream.tellp() != -1);
    stream.seekp(static_cast<size_t>(stream.tellp()) + s);
    archive<<"0000";
    archive<< static_cast<char>(0xff);
    stream.flush();
    system("echo \\$ xxd Boost && xxd Boost");
    /*! Expected output:
     * 00000000: 3132 3333 3333 0000 0000 0002 0000 0000  123333.............hi
     * ...
     * 000004e0: 3333 3333 3333 33                        3333333
     */


    return 0;
}
