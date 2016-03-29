//
//  TSValueObject.hpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/20/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSValue_hpp
#define TSValue_hpp

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

class TSValue {
public:
    virtual ~TSValue();
    
    virtual std::shared_ptr<TSValue> copy() const = 0;
    virtual std::string description() const = 0;
    virtual std::string type() const = 0;
    
    template <typename T>
    bool isType() {
        return type() == std::string(typeid(T).name());
    }
    
    typedef std::vector< std::shared_ptr<TSValue> > ArrayType;
    
    /// A value is an `array` IFF
    ///     type == TSValue::ArrayType
    bool isArray();
};

template<typename T>
class TSSpecificValue : public TSValue {
public:
    const std::string type_name;

    TSSpecificValue(const T & val) : type_name(typeid(T).name()) {
        value_ptr = std::shared_ptr<T>(new T(val));
        *value_ptr = val;
    }

    virtual ~TSSpecificValue() {}
    virtual const T & get() const {
        return *value_ptr;
    }

    virtual std::shared_ptr<TSValue> copy() const {
        return std::shared_ptr<TSValue>(new TSSpecificValue<T>(get()));
    }

    virtual std::string description() const {
        return parse(get());
    }
    
    virtual std::string parse(const T & val) const {return "(" + type_name + ")";}

    virtual std::string type() const {
        return type_name;
    }

private:
    std::shared_ptr<T> value_ptr;
};

///
class TSBoolValue : public TSSpecificValue<bool> {
public:
    TSBoolValue(const bool & val) : TSSpecificValue<bool>(val) {}
    virtual std::string parse(const bool & val) const {
        return (val? "true" : "false");
    }
};

///
class TSIntValue : public TSSpecificValue<int> {
public:
    TSIntValue(const int & val) : TSSpecificValue<int>(val) {}
    virtual std::string parse(const int & val) const {
        return std::to_string(val);
    }
};

///
class TSDoubleValue : public TSSpecificValue<double> {
public:
    TSDoubleValue(const double & val) : TSSpecificValue<double>(val) {}
    virtual std::string parse(const double & val) const {
        return std::to_string(val);
    }
};

///
class TSStringValue : public TSSpecificValue<std::string> {
public:
    TSStringValue(const std::string & val) : TSSpecificValue<std::string>(val) {}
    
    virtual std::string
    parse(const std::string & val) const {
        return val;
    }
};

///
class TSValueArray : public TSSpecificValue< TSValue::ArrayType > {
public:
    TSValueArray(const TSValue::ArrayType & val)
     : TSSpecificValue< TSValue::ArrayType >(val) {}

    virtual std::string
    parse(const TSValue::ArrayType & val) const {
        std::string res = "[";
        for (size_t k = 0; k < val.size(); k++) {
            res += val[k]->description() + " ";
        }
        return res + "]";
    }
};

template <typename T>
const T & resolve_value(std::shared_ptr<TSValue> val) {
    
    std::shared_ptr< TSSpecificValue<T> > specificValue
     = std::dynamic_pointer_cast< TSSpecificValue<T> >(val);
            
    if (specificValue == nullptr) {
        throw std::runtime_error(std::string("value_map: requested type T = ")
         + typeid(T).name() + " does not match the one given by name value with"
         + " type \"" + val->type() + "\"");
    }
    else {
        return specificValue->get();
    }
}

///
struct TSValueMap {
private:
    std::map< std::string, std::shared_ptr<TSValue> > values;

public:
    
    TSValueMap();
    TSValueMap(const TSValueMap & other);
    
    template<typename T>
    void set(std::string name, const T & val) {
        values[name] = std::shared_ptr<TSValue>(new TSSpecificValue<T>(val));
    }
    
    void setRaw(std::string name, std::shared_ptr<TSValue> val);

    template<typename T>
    const T & get(std::string name) const {
        if (values.count(name) != 0) {
            std::shared_ptr<TSValue> base = values.at(name);
            return resolve_value<T>(base);
        }
        else {
            throw std::runtime_error("value_map: no value given by the name \""
             + name + "\" could be found.");
        }
    }
    
    std::shared_ptr<TSValue> getRaw(std::string name);
    
    ///
    /// Allow consumers of this struct to use it sort of like a map
    ///
    
    /// Returns whether there exists a value for `name`
    bool contains(std::string name);
    
    template <typename T>
    bool has(std::string name) {
        return contains(name) && (*this)[name].isType<T>();
    }
    
    /// checks to see of a value of type T with `name` exists, and then
    /// calls `callback` (any sort of call-able thing that can take a T).
    /// Returns the result of this->has<T>(name).
    template <typename T, typename C>
    bool resolveIfPresent(std::string name, C callback) {
        bool hasIt = has<T>(name);
        if (hasIt) {
            callback(get<T>(name));
        }
        return hasIt;
    }
    
    bool empty() const {
        return values.empty();
    }
    
    /// Removes all entries in the map
    void clear();
    /// Returns `true` IFF `name` was removed, `false` is `name` DNE in map
    bool remove(std::string name);
    
    /// Adds all other elements, including ones already set in `other`
    void addAll(const TSValueMap & other);
    
    /// Access
    struct Accessor {
        TSValueMap & map;
        std::string name;
        
        Accessor(TSValueMap & m, std::string n) : map(m), name(n) {}
        
        template <typename T>
        const T & operator= (T const & rhs) {
            map.set(name, rhs);
            return map.get<T>(name);
        }
        
        template <typename T>
        operator T const & () {return map.get<T>(name);}
        
        bool exists() {
            return map.values.count(name) != 0;
        }
        
        template <typename T>
        bool isType() {
            return map.values[name]->isType<T>();
        }
        
        template <typename T>
        const T & as() {return map.get<T>(name);}
        
        std::shared_ptr<TSValue> rawValue() {return map.values[name];}
        
        Accessor
        operator [] (std::string name) {
            auto innerMap = as<TSValueMap>();
            return Accessor(innerMap, name);
        }
    };
    
    Accessor operator[] (std::string name) const;
    Accessor operator[] (std::string name);
    
    /// Iteration
    typedef std::map< std::string,std::shared_ptr<TSValue> >::iterator iterator;
    typedef std::map< std::string,std::shared_ptr<TSValue> >::const_iterator const_iterator;
    
    iterator begin();
    const_iterator begin() const;
    
    iterator end();
    const_iterator end() const;
    
    ///
    std::string description() const;
};

#endif /* TSValue_hpp */
