./#include <iostream>
#include <cstring>
#include <reverseengine/common.hh>


template <class T>
class HEX__2 {
public:
    HEX__2(T &value)
    : value(value) { }

    HEX__2 size(size_t s) {
        value_size = s;
        return *this;
    }

    std::string
    str() const {
        uint8_t *buffer = (uint8_t*)(&value);
        char converted[value_size * 2 + 1];
        if (endianess == std::endian::big)
            for(size_t i = 0; i < value_size; ++i) {
                sprintf(&converted[i*2], "%02X", buffer[i]);
            }
        else
            for(size_t i = 0; i < value_size; ++i) {
                sprintf(&converted[i*2], "%02X", buffer[value_size-1-i]);
            }
        return converted;
    }

    friend std::ostream& operator<<(std::ostream& os, const HEX__2& hex__2) {
        return os<<hex__2.str();
    }

private:
    std::endian endianess = std::endian::native;
    size_t value_size = sizeof(T);
    T value;
};



void main2 () {

    using namespace std;
    {
        uint8_t a = 0x10;
        cout<<HEX__2(a).size(4)<<endl;
        if (HEX__2(a).str() != "10") throw std::runtime_error("");
    }{
        int32_t a = -128;
        cout<<HEX__2(a)<<endl;
        if (HEX__2(a).str() != "FFFFFF80") throw std::runtime_error("");
    }{
        uint64_t a = 0x0000000000000102;
        cout<<HEX__2(a).size(2)<<endl;
        if (HEX__2(a).str() != "0000000000000102") throw std::runtime_error("");
    }{
        double a = 1;
        cout<<HEX__2(a)<<endl;
        if (HEX__2(a).str() != "3FF0000000000000") throw std::runtime_error("");
    }
    cout<<"------------------"<<endl;
    {
        uint64_t a = 0;
        cout<<HEX__2(a)<<endl;
        if (HEX__2(a).str() != "0000000000000000") throw std::runtime_error("");
    }
}







/** @arg what: any number
 * @return: string number represented as hex */
template <typename T, std::endian endianess = std::endian::native>
std::string HEX__1(const T& value, bool showbase = false, bool trim_zero = false, size_t value_size = sizeof(T))
{
    using namespace std;
    if UNLIKELY(value_size > sizeof(T))
        throw std::runtime_error("HEX__1: value_size > sizeof(T)");
    uint8_t *buffer = (uint8_t*)(&value);
    char converted[value_size * 2 + 1];
    if UNLIKELY(endianess == std::endian::big)
        for(size_t i = 0; i < value_size; ++i) {
            sprintf(&converted[i*2], "%02X", buffer[i]);
        }
    else
        for(size_t i = 0; i < value_size; ++i) {
            sprintf(&converted[i*2], "%02X", buffer[value_size-1-i]);
        }
    std::string r(converted);
    if (trim_zero)
        r.erase(0, min(r.find_first_not_of('0'), r.size()-1));
    if (showbase)
        r = "0x"+std::move(r);
    return r;
}

void main1 () {

    using namespace std;
    {
        uint8_t a = 0x10;
        cout<<HEX__1(a)<<endl;
        if (HEX__1(a) != "10") throw std::runtime_error("");
    }
    {
        int32_t a = -128;
        cout<<HEX__1(a)<<endl;
        if (HEX__1(a) != "FFFFFF80") throw std::runtime_error("");
    }
    {
        uint64_t a = 0x0000000000000102;
        cout<<endl;
        cout<<HEX__1(a,0,0)<<endl;
        cout<<HEX__1(a,1,0)<<endl;
        cout<<HEX__1(a,0,1)<<endl;
        cout<<HEX__1(a,1,1)<<endl;
        cout<<endl;
        if (HEX__1(a) != "0000000000000102") throw std::runtime_error("");
    }
    {
        double a = 1;
        cout<<HEX__1(a)<<endl;
        if (HEX__1(a) != "3FF0000000000000") throw std::runtime_error("");
    }
    cout<<"------------------"<<endl;
    {
        uint64_t a = 0;
        cout<<HEX__1(a)<<endl;
        if (HEX__1(a) != "0000000000000000") throw std::runtime_error("");
    }
}

int main() {
    main1();
    //main2();
    return 0;
}