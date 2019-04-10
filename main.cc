#include <utility>

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <zconf.h>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <regex>


//using namespace std;
//
//class reader_i {
//public:
//    reader_i() = default;
//    ~reader_i() = default;
//
//    virtual int read() = 0;
//};
//
//class reader1 : public reader_i {
//public:
//    using reader_i::reader_i;
//
//    int read() override {
//        cout<<"1"<<endl;
//        return 1;
//    }
//};
//class reader2 : public reader_i {
//public:
//    using reader_i::reader_i;
//
//    int read() override {
//        cout<<"2"<<endl;
//        return 2;
//    }
//};
//
//template <class B>
//class cached_reader {
//public:
//    explicit cached_reader(B* base)
//    : base(base) {}
//
//    template<class T = B, typename std::enable_if<std::is_base_of<reader1, T>::value>::type* = nullptr>
//    int read() {
//        return base->read();
//    }
//
//    template<class T = B, typename std::enable_if<!std::is_base_of<reader1, T>::value>::type* = nullptr>
//    int read() {
//        return base->read();
//    }
//
//private:
//    B* base;
//};


//int main(int argc, char* argv[]) {
//    using namespace std;
//
//    reader1 r1;
//    reader2 r2;
//
//    cached_reader c1(&r1);
//    cached_reader c2(&r2);
//
//    c1.read();
//    c2.read();
//
//    cout<<" No Errors"<<endl;
//    return 0;
//}


//class Timer {
//public:
//    explicit Timer(std::string what = "Timer")
//    : m_what(std::move(what)), m_tp(std::chrono::high_resolution_clock::now()) {}
//
//    ~Timer() {
//        clog<<m_what<<": done in "<<std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - m_tp).count()<<" seconds"<<endl;
//    }
//
//private:
//    std::string m_what;
//    std::chrono::high_resolution_clock::time_point m_tp;
//};



#include <r_core.h>
#include <r_types.h>
#include <r_util.h>
#include <r_bin.h>
#include <r_io.h>


int anal_all = 0;   // radiff2.c -A(=anal_all++)
bool useva = false; // radiff2.c -p
static RList *evals = NULL;
const char *runcmd = NULL; // radiff2.c -G run cmd before any RCore creation
static RCore *c = NULL;


std::string
get_symbols(RCore *c) {
    RList *list = r_bin_get_sections(c->bin);
    RListIter *iter;
    std::ostringstream ss;

    if (!list)
        return "";

    // Nm Paddr       Size Vaddr      Memsz Perms Name
    // 00 0x00000000     0 0x00000000     0 ----
    // 01 0x000001c8    36 0x000001c8    36 -r-- .note.gnu.build_id
    for (iter = list->head; iter; iter = iter->n) {
        RBinSection *sec = static_cast<RBinSection *>(iter->data);
        ss << std::showbase
           << std::hex << sec->paddr << std::dec << " "
           << sec->size << " "
           << std::hex << sec->vaddr << std::dec << " "
           << sec->vsize << " "
           << sec->perm << " "
           << sec->name;
        if (iter->n)
            ss<<'\n';
    }

    return ss.str();
}


std::string
get_sections(RCore *c) {
    RList *list = r_bin_get_sections(c->bin);
    RListIter *iter;
    std::ostringstream ss;

    if (!list)
        return "";

    // Nm Paddr       Size Vaddr      Memsz Perms Name
    // 00 0x00000000     0 0x00000000     0 ----
    // 01 0x000001c8    36 0x000001c8    36 -r-- .note.gnu.build_id
    for (iter = list->head; iter; iter = iter->n) {
        RBinSection *sec = static_cast<RBinSection *>(iter->data);
        ss << std::showbase
           << std::hex << sec->paddr << std::dec << " "
           << sec->size << " "
           << std::hex << sec->vaddr << std::dec << " "
           << sec->vsize << " "
           << sec->perm << " "
           << sec->is_data << " "
           << sec->is_segment << " "
           << sec->name;
        if (iter->n)
            ss<<'\n';
    }

    return ss.str();
}


/// from libr/main/radiff2.c
std::string
get_strings(RCore *c) {
    RList *list = r_bin_get_strings(c->bin);
    RListIter *iter;
    std::ostringstream ss;

    if (list)
        for (iter = list->head; iter; iter = iter->n) {
            ss << static_cast<RBinString *>(iter->data)->string;
            if (iter->n)
                ss<<'\n';
        }
    return ss.str();
}

/// from libr/main/radiff2.c
static RCore *opencore(const char *file) {
    RListIter *iter;
    const ut64 baddr = UT64_MAX;
    const char *e;
    RCore *c = r_core_new();
    if (!c) {
        return NULL;
    }
    r_core_loadlibs(c, R_CORE_LOADLIBS_ALL, NULL);
    r_config_set_i(c->config, "io.va", useva);
    r_config_set_i(c->config, "scr.interactive", false);
    if (evals)
        for (iter = evals->head; iter && (e = static_cast<const char *>(iter->data), 1); iter = iter->n) {
            r_config_eval(c->config, e);
        }
    if (file) {
#if __WINDOWS__
        file = r_acp_to_utf8 (file);
#endif
        if (!r_core_file_open(c, file, 0, 0)) {
            r_core_free(c);
            return NULL;
        }
        (void) r_core_bin_load(c, NULL, baddr);
        (void) r_core_bin_update_arch_bits(c);

        // force PA mode when working with raw bins
        if (r_list_empty (r_bin_get_sections(c->bin))) {
            r_config_set_i(c->config, "io.va", false);
        }

        if (anal_all) {
            const char *cmd = "aac";
            switch (anal_all) {
                case 1:
                    cmd = "aaa";
                    break;
                case 2:
                    cmd = "aaaa";
                    break;
            }
            r_core_cmd0(c, cmd);
        }
        if (runcmd) {
            r_core_cmd0(c, runcmd);
        }
        // generate zignaturez?
        // if (zignatures) {
        //     r_core_cmd0 (c, "zg");
        // }
        r_cons_flush();
    }
    // TODO: must enable io.va here if wanted .. r_config_set_i (c->config, "io.va", va);
    return c;
}

int main(int argc, char *argv[]) {
    using namespace std;
    namespace sfs = std::filesystem;
    {
        std::fstream f("/proc/self/oom_score_adj", std::ios::out | std::ios::binary);
        f << "1000";
    }

    // TODO: почему мейн в пизде какой-то? Только в арче????
    // std::fstream f("/proc/self/maps", std::ios::in | std::ios::binary);
    // std::cout<<f.rdbuf()<<std::endl;
    // std::cout<<"main: "<< hex<<showbase<<reinterpret_cast<void *>(main)<<endl;
    // std::cout<<"&main: "<< hex<<showbase<<reinterpret_cast<void *>(&main)<<endl;

    // TODO: доделать HEX
    // std::string str = "0";
    // str.erase(0, min(str.find_first_not_of('0'), str.size()-1));
    // std::cout<<"str: "<<str<<endl;
    int sza;

    const char *filename = "/home/user/.local/share/Steam/steamapps/common/Counter-Strike Global Offensive/bin/linux64/libtier0_client.so";
    {
        // Init
        evals = r_list_newf(NULL);
    }
    {
        // Body

        c = opencore(filename);
        if (!c) {
            eprintf ("Cannot open '%s'\n", r_str_get(filename));
        }


        RBin *bin = r_bin_new();

        cout << get_strings(c) << endl << endl << endl;
        cout << get_sections(c) << endl;
    }
    {
        // END
        r_core_free(c);
    }



    // {
    //     // std::filesystem::path file = "/proc/self/exe";
    //     std::filesystem::path file = "/home/user/.local/share/Steam/steamapps/common/Counter-Strike Global Offensive/csgo_linux64";
    //     std::fstream f(file, std::ios::in | std::ios::binary);
    //
    //     FILE *fp = fopen(file.c_str(), "r");
    //     cout<<"ll "<<file.c_str()<<endl;
    //
    //     RBin *bin = r_bin_new();
    //
    //     opt.xtr_idx = 0;
    //     opt.pluginname = file.c_str();
    //     if (!r_bin_open_io(bin, file.c_str(), &opt)) {
    //         throw std::runtime_error("!r_bin_open");
    //     }
    //
    //     // r_bin_get_symbols
    //
    //     RBinInfo *info = r_bin_get_info(bin);
    //     cout<<"file: "<<info->file<<endl;
    //
    //     r_bin_free(bin);
    // }

    cout << "No Errors" << endl;
    return 0;
}
