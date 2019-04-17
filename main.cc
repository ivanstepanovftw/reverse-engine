#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <memory>

#include <r_core.h>
#include <r_types.h>
#include <r_util.h>
#include <r_bin.h>
#include <r_io.h>
#include <regex>
#include <cmath>



class Timer {
public:
   explicit Timer(std::string what = "Timer")
   : m_what(std::move(what)), m_tp(std::chrono::high_resolution_clock::now()) {}

   ~Timer() {
       std::clog<<m_what<<": done in "<<std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - m_tp).count()<<" seconds"<<std::endl;
   }

private:
   std::string m_what;
   std::chrono::high_resolution_clock::time_point m_tp;
};




namespace sfs = std::filesystem;
// namespace bio = boost::iostreams;


int main(int argc, char *argv[]) {
    int n=400000,  m=1000;
    double x=0,y=0;
    std::vector<double> shifts(n,0);

    {
        Timer __t("none");
        for (int j=0; j<n; j++) {
            double r=0.0;
            for (int i=0; i < m; i++) {
                double rand_g1 = std::cos(i/double(m));
                double rand_g2 = std::sin(i/double(m));

                x += rand_g1;
                y += rand_g2;
                r += sqrt(rand_g1*rand_g1 + rand_g2*rand_g2);
            }
            shifts[j] = r / m;
        }
    }

    {
        Timer __t("omp");
        #pragma omp parallel for
        for (int j=0; j<n; j++) {
            double r=0.0;
            for (int i=0; i < m; i++) {
                double rand_g1 = std::cos(i/double(m));
                double rand_g2 = std::sin(i/double(m));

                x += rand_g1;
                y += rand_g2;
                r += sqrt(rand_g1*rand_g1 + rand_g2*rand_g2);
            }
            shifts[j] = r / m;
        }
    }

    {
        Timer __t("omp+");
        #pragma omp parallel for reduction(+:x,y)
        for (int j=0; j<n; j++) {
            double r=0.0;
            for (int i=0; i < m; i++) {
                double rand_g1 = std::cos(i/double(m));
                double rand_g2 = std::sin(i/double(m));

                x += rand_g1;
                y += rand_g2;
                r += sqrt(rand_g1*rand_g1 + rand_g2*rand_g2);
            }
            shifts[j] = r / m;
        }
    }

    std::cout << *std::max_element( shifts.begin(), shifts.end() ) << std::endl;

    std::cout << "No Errors" << std::endl;
    return 0;
}
