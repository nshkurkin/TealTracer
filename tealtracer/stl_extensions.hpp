//
//  stl_extensions.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/2/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef stl_extensions_hpp
#define stl_extensions_hpp

#include <map>

///
template <typename First, typename Second>
void set_map(std::map<First,Second> & map);
///
template <typename First, typename Second>
void set_map(std::map<First,Second> & map, const First & f, const Second & s);
///
template <typename First, typename Second, typename... Args>
void set_map(std::map<First,Second> & map, const First & f, const Second & s, Args... args);
///
template <typename First, typename Second>
std::map<First, Second> make_map(const First & f, const Second & s);
///
template <typename First, typename Second, typename... Args>
std::map<First, Second> make_map(const First & f, const Second & s, Args... args);

#include <vector>

///
template <typename T>
void add_vector(std::vector<T> & vec);
///
template <typename T>
void add_vector(std::vector<T> & vec, const T & t);
///
template <typename T, typename... Args>
void add_vector(std::vector<T> & vec, const T & t, Args... args);
///
template <typename T, typename... Args>
std::vector<T> make_vector(Args... args);

/////////////////////////////////////////////////////////////////////////////////

///
template <typename First, typename Second>
void set_map(std::map<First,Second> & map) {}

///
template <typename First, typename Second>
void set_map(std::map<First,Second> & map, const First & f, const Second & s) {
    map[f] = s;
}

///
template <typename First, typename Second, typename... Args>
void set_map(std::map<First,Second> & map, const First & f, const Second & s, Args... args) {
    map[f] = s;
    set_map<First, Second>(map, args...);
}

///
template <typename First, typename Second>
std::map<First, Second> make_map(const First & f, const Second & s) {
    std::map<First, Second> map;
    set_map<First, Second>(map, f, s);
    return map;
}

///
template <typename First, typename Second, typename... Args>
std::map<First, Second> make_map(const First & f, const Second & s, Args... args) {
    std::map<First, Second> map;
    set_map<First, Second>(map, f, s, args...);
    return map;
}

/////////////////////////////////////////////////////////////////////////////////

///
template <typename T>
void add_vector(std::vector<T> & vec) {}

///
template <typename T>
void add_vector(std::vector<T> & vec, const T & t) {
    vec.push_back(t);
}

///
template <typename T, typename... Args>
void add_vector(std::vector<T> & vec, const T & t, Args... args) {
    vec.push_back(t);
    add_vector<T>(vec, args...);
}

///
template <typename T, typename... Args>
std::vector<T> make_vector(Args... args) {
    std::vector<T> vec;
    add_vector<T>(vec, args...);
    return vec;
}

#endif /* stl_extensions_hpp */
