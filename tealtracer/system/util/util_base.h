///
/// util_base.h
/// ------------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_base__
#define ____util_base__

#include <Eigen/Dense>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

///
/// Below is a pretty wonky way of defining a "Printable" interface (has a
/// "description" method on the struct/class) and an overload of the <<
/// operator on an ostream to automatically unpackage the "description" from
/// valid types. So:
///
/// interface Printable {
///    std::string description();
/// };
///
/// It is useful to note that values are passed by reference to the << operator
/// and will automatically do a dynamic lookup through the use of a pointer. This
/// avoids some clunkiness and allows descriptions to be on structs and classes.
///

/// Pulled from http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence
#define HAS_MEM_FUNC(func, name)                                        \
    template<typename T, typename Sign>                                 \
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename U, U> struct type_check;                     \
        template <typename _1> static yes &chk(type_check<Sign, &_1::func > *); \
        template <typename   > static no  &chk(...);                    \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }

template<bool C, typename T = void>
struct enable_if {
    typedef T type;
};

template<typename T>
struct enable_if<false, T> { };

HAS_MEM_FUNC(description, Printable);

template<typename PrintableType>
typename enable_if<Printable<PrintableType,
std::string(PrintableType::*)()>::value,
std::ostream& >::type
operator<< (std::ostream &out, PrintableType &toPrint) {
    out << (&toPrint)->description();
    return out;
}


///
/// Building off of the Printable "interface", you can now add a printable
/// thing to a string to make a concatenated string with it. So:
///
///   std::string str = "Hello " + printableThing;
///

template<typename PrintableType>
typename enable_if<Printable<PrintableType,
std::string(PrintableType::*)()>::value,
std::string >::type
operator+ (const std::string str, PrintableType &toPrint) {
    return str + (&toPrint)->description();
}

template<typename PrintableType>
typename enable_if<Printable<PrintableType,
std::string(PrintableType::*)()>::value,
std::string >::type
operator+ (PrintableType &toPrint, const std::string str) {
    return (&toPrint)->description() + str;
}

///
/// Allows us to append eigen matrices to strings in a nice way. This includes
/// automatically takign the transpose of vertical vectors for better stdout
/// printing results.
///

template<typename Type, int x, int y, int z, int w, int c>
std::string operator+ (const std::string & str,
                       Eigen::Matrix<Type, x, y, z, w, c> mat) {
    Eigen::IOFormat fmt(4, 0, ", ", "\n", "[", "]");
    std::ostringstream strFormatter;
    if (y == 1)
        strFormatter << mat.transpose().format(fmt);
    else
        strFormatter << mat.format(fmt);
    return str + strFormatter.str();
}

template<typename Type, int x, int y, int z, int w, int c>
std::string operator+ (Eigen::Matrix<Type, x, y, z, w, c> mat,
                       const std::string & str) {
    Eigen::IOFormat fmt(4, 0, ", ", "\n", "[", "]");
    std::ostringstream strFormatter;
    if (y == 1)
        strFormatter << mat.transpose().format(fmt);
    else
        strFormatter << mat.format(fmt);
    return strFormatter.str() + str;
}

///
/// The following gives a generic way of adding a string to any raw type.
///

#define DEFINE_STR_OP(op, Type) \
   std::string operator op (const std::string str, Type value);\
   std::string operator op (Type value, const std::string str)

#define IMPL_STR_OP(op, Type) \
   std::string operator op (const std::string str, Type value) { \
      std::ostringstream strFormatter; \
      strFormatter << value; \
      return str + strFormatter.str(); \
   } \
   std::string operator op (Type value, const std::string str) { \
      std::ostringstream strFormatter; \
      strFormatter << value; \
      return strFormatter.str() + str; \
   }

DEFINE_STR_OP(+, double);

///
///
/// Stringifying the contents of std::vector containers
///
///

template<typename T>
std::string operator+ (const std::string & str, std::vector<T> vec) {
    std::string vecAsStr = "[";
    for (int i = 0; i < (int) vec.size(); i++) {
        vecAsStr += std::string("") + vec[i];
        if (i < ((int) vec.size()) - 1)
            vecAsStr += ", ";
    }
    vecAsStr += "]";
    return str + vecAsStr;
}

template<typename T>
std::string operator+ (std::vector<T> vec, const std::string & str) {
    std::string vecAsStr = "[";
    for (int i = 0; i < vec.size(); i++) {
        vecAsStr += std::string("") + vec[i];
        if (i < vec.size() - 1)
            vecAsStr += ", ";
    }
    vecAsStr += "]";
    return vecAsStr + str;
}

template<typename T>
bool operator== (const std::vector<T> & a, const std::vector<T> & b) {
    int which = -1;
    bool matches = a.size() == b.size();
    while (matches && ++which < (int) a.size())
        matches = a[which] == b[which];
    return matches;
}

namespace util {
    template <typename T, int kNumComps>
    struct EigenVecLess {
        typedef Eigen::Matrix<T, kNumComps, 1> Vec;
        
        bool operator()( const Vec& lhs, const Vec& rhs ) const {
            bool leftIsLess = true;
            bool shouldBreak = false;
            int comp = -1;
            
            while (!shouldBreak && ++comp < kNumComps) {
                if (lhs[comp] == rhs[comp])
                    leftIsLess = false;
                // Comps match, not less, but go on
                else if (lhs[comp] < rhs[comp]) {
                    shouldBreak = leftIsLess = true;
                }
                else {
                    shouldBreak = true;
                    leftIsLess = false;
                }
            }
            return leftIsLess;
        }
    };
}

///
///
/// General purpose logging function
///
///

namespace util {
    /// Used to create a log statement in stdout
    void log(std::string toLog,
             const std::string file = __FILE__, int line = __LINE__);
    
    /// Returns the string after the first occurence of `t` in `toTrim`
    std::string trimLeft(std::string toTrim, const std::string & t);
    /// Returns the string before the first occurence of `t` in `toTrim`
    std::string trimRight(std::string toTrim, const std::string & t);
}

/// Use this to log messages with the added bonus of the "+" operator.
#define UTIL_LOG(statement) \
 util::log(std::string("") + statement, __FILE__, __LINE__)
#define LOG(statement) UTIL_LOG(statement)
/// Use this to quickly print out where you are
#define HERE() util::log("here", __FILE__, __LINE__)

#define UTIL_ASSERT(cnd) {\
    if (!(cnd)) {\
        UTIL_LOG("Assertion Failed: " #cnd);\
        exit(-1);\
    }\
}

#define UTIL_ERROR(statement) {\
    UTIL_LOG("Error: " + statement);\
    exit(-1);\
}

#define UTIL_WHAT_IS(thing) UTIL_LOG( #thing ": " + thing)

#endif // ____util_base__
