//
//  TSDoubleBufferedValue.hpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/25/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSDoubleBufferedValue_hpp
#define TSDoubleBufferedValue_hpp

///
template <typename T>
struct TSDoubleBufferedValue {
public:
    ///
    TSDoubleBufferedValue() : value_(T()), nextValue_(T()), changed_(false) {}
    ///
    TSDoubleBufferedValue(const T & value) : value_(value), nextValue_(value), changed_(false) {}
    ///
    TSDoubleBufferedValue(const TSDoubleBufferedValue<T> & buffer) {
        value_ = buffer.value_;
        nextValue_ = buffer.nextValue_;
        changed_ = buffer.changed_;
    }
    
    
    ///
    const T & currentValue() const {
        return value_;
    }
    
    ///
    const T & nextValue() const {
        return nextValue_;
    }
    
    ///
    bool swap() {
        bool wasChanged = changed_;
        if (wasChanged) {
            value_ = nextValue_;
            changed_ = false;
        }
        return wasChanged;
    }
    
    ///
    TSDoubleBufferedValue<T> & operator=(const T & value) {
        nextValue_ = value;
        changed_ = true;
        return *this;
    }
    
    ///
    void overwrite(const T & value) {
        value_ = value;
        nextValue_ = value;
        changed_ = false;
    }
    
    ///
    operator T const & () const {
        return value_;
    }
    
    ///
    T * operator->() {
        changed_ = true;
        return &nextValue_;
    }
    ///
    T * operator->() const {
        return &nextValue_;
    }
    
    ///
    T & operator*() {
        changed_ = true;
        return nextValue_;
    }
    ///
    T & operator*() const {
        return nextValue_;
    }
    
private:
    ///
    T value_;
    ///
    T nextValue_;
    ///
    bool changed_;
};

#endif /* TSDoubleBufferedValue_hpp */
