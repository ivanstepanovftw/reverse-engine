#include <reverseengine/core.hh>
#include <reverseengine/r2_wrapper.hh>
#include <reverseengine/scanner.hh>

int main() {
    {
        std::vector<int> a;
        for(int b : a) {
            cout<<"b: "<<b<<endl;
        }
    }
    {
        RE::ByteMatches m0;
        RE::ByteMatches::iterator __begin = m0.begin();
        RE::ByteMatches::iterator __end = m0.end();
        for (; __begin != __end; ++__begin) {
            cout<<"aaaa!!!!!!!!!"<<endl;
        }
    }
    {
        RE::ByteMatches m0;
        for(RE::value_t a0 : m0) {
            cout<<"bbbbbb!!!"<<a0<<endl;
        }
    }
}
