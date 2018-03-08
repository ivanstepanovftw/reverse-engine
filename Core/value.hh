//
// Created by root on 06.03.18.
//

#ifndef RE_CALCULATOR_HH
#define RE_CALCULATOR_HH

#include <iostream>
#include <cstring>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <cmath>
#include <functional>
#include <cstdlib>
#include <sstream>
#include <regex>
#include <cstddef>
#include <climits>

template<class T>
struct Operator {
    Operator(const char *s, std::function<T(T,T)> f, unsigned short arity):_symbol(s),_function(f),_arity(arity){}
    T Fire(T a,T b){return _function(a,b);}
    
    const char* _symbol;
    std::function<T(T,T)> _function;
    unsigned short _arity;
    
};

struct ExpressionInterval{
    ExpressionInterval(unsigned i, unsigned j):_start(i),_end(j){}
    void Set(unsigned i, unsigned j){_start=i;_end=j;}
    unsigned _start;
    unsigned _end;
};

class Parser
{
public:
    Parser();
    ~Parser();
    
    float ParseInput(char *in);
    
    bool StrCompare(char *one, const char *two, unsigned numberOfChars);

private:
    
    void NewInput(char *input);
    
    float BuildSubTree(ExpressionInterval ei,int delimiter);
    float Leaf(ExpressionInterval ei);
    
    
    std::vector<Operator<float>> _operators;
    
    char *_inputString;
};



class lexical_cast_sign : public std::bad_cast
{
public:
    lexical_cast_sign() noexcept
            : source(&typeid(void)), target(&typeid(void)) {}
    
    virtual const char *what() const noexcept
    {
        return wat.c_str();
    }
    
    virtual ~lexical_cast_sign() noexcept {}
    
    lexical_cast_sign(const std::type_info &source_type_arg, 
                     const std::type_info &target_type_arg) noexcept
            : source(&source_type_arg)
            , target(&target_type_arg)
    { }
    
    const std::type_info &source_type() const noexcept
    {
        return *source;
    }
    
    const std::type_info &target_type() const noexcept
    {
        return *target;
    }

private:
    const std::type_info *source;
    const std::type_info *target;
    std::string wat = "bad lexical cast: source type value could not be interpreted as target";
};

class lexical_cast_range : public std::bad_cast
{
public:
    lexical_cast_range() noexcept
            : source(&typeid(void)), target(&typeid(void)) {}
    
    virtual const char *what() const noexcept
    {
        return wat.c_str();
    }
    
    virtual ~lexical_cast_range() noexcept {}
    
    lexical_cast_range(const std::type_info &source_type_arg, 
                     const std::type_info &target_type_arg)
    noexcept
            : source(&source_type_arg)
            , target(&target_type_arg)
    { }
    
    const std::type_info &source_type() const noexcept
    {
        return *source;
    }
    
    const std::type_info &target_type() const noexcept
    {
        return *target;
    }

private:
    const std::type_info *source;
    const std::type_info *target;
    std::string wat = "bad lexical cast: source type value could not be interpreted as target";
};


using namespace std;

template<typename T>
T lexical_cast(const char *nptr, bool *negative = 0, char **endptr = 0, int base = 0)
{
    const char *s = nptr;
    int c;
    T cutoff;
    int neg = 0, any, cutlim;
    T output;
    
    // skip spaces, set sign, set base
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        if (!std::numeric_limits<T>::is_signed) {
//            std::clog<<"bad lexical cast: found minus sig but type is unsigned"<<std::endl;
            throw lexical_cast_sign();
        }
        neg = 1;
        if (negative != 0)
            *negative = static_cast<bool>(neg);
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    
    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = std::numeric_limits<T>::max() / (T)base;
    cutlim = std::numeric_limits<T>::max() % (T)base;
//    std::clog<<"["<<+std::numeric_limits<T>::min()<<", "<<+std::numeric_limits<T>::max()<<"]"<<std::endl;
    for(output = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || output > cutoff || (output == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            output *= base;
            output += c;
        }
    }
    if (any < 0) {
        throw lexical_cast_range();
//        output = is_unsigned<T>()?type_max<T>():(neg?type_min<T>():type_max<T>());
//        errno = ERANGE;
    } else if (neg) {
        output = -output;
    }
    if (endptr != 0)
        *endptr = (char *) (any ? s - 1 : nptr);
    return (output);
}

#endif //RE_CALCULATOR_HH
