//
//  TSValue.cpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/20/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TSValue.hpp"

///
TSValue::~TSValue() {}

bool
TSValue::isArray() {
    return type() == typeid(std::vector< std::shared_ptr<TSValue> >).name();
}

///
TSValueMap::TSValueMap() {}
TSValueMap::TSValueMap(const TSValueMap & other) {
    for (auto itr = other.begin(); itr != other.end(); itr++) {
        values[itr->first] = itr->second->copy();
    }
}

///
bool
TSValueMap::contains(std::string name) {
    return values.count(name) != 0;
}

void
TSValueMap::clear() {
    values.clear();
}

bool
TSValueMap::remove(std::string name) {
    return values.erase(name) == 1;
}

void
TSValueMap::addAll(const TSValueMap & other) {
    for (auto itr = other.values.begin(); itr != other.values.end(); itr++) {
        this->values[itr->first] = itr->second;
    }
}

///
void
TSValueMap::setRaw(std::string name, std::shared_ptr<TSValue> val) {
    values[name] = val;
}

std::shared_ptr<TSValue>
TSValueMap::getRaw(std::string name) {
    return values[name];
}

///
TSValueMap::Accessor
TSValueMap::operator[] (std::string name) const {
    return Accessor((TSValueMap &)*this, name);
}

TSValueMap::Accessor
TSValueMap::operator[] (std::string name) {
    return Accessor(*this, name);
}

///
TSValueMap::iterator
TSValueMap::begin() {
    return values.begin();
}

TSValueMap::const_iterator
TSValueMap::begin() const {
    return values.begin();
}

TSValueMap::iterator
TSValueMap::end() {
    return values.end();
}

TSValueMap::const_iterator
TSValueMap::end() const {
    return values.end();
}

///
std::string
TSValueMap::description() const {
    std::string str = "[";
    for (auto itr = begin(); itr != end(); itr++) {
        str += itr->first + ":" + itr->second->description() + " ";
    }
    return str + "]";
}
