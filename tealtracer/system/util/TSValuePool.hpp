//
//  TSValuePool.hpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/20/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSValuePool_hpp
#define TSValuePool_hpp

#include <cassert>
#include <vector>

typedef int TSValueRef;
#define TSValueNullRef ((TSValueRef)-1)

///
template <typename T>
class TSValuePool {
private:
    ///
    std::vector<T> values_;
    ///
    int firstFreeIdx_;
    
public:

    ///
    TSValuePool(int capacity, const T & fillValue) {
        for (int i = 0; i < capacity; i++) {
            values_.push_back(fillValue);
        }
        
        firstFreeIdx_ = 0;
    }
    
    ///
    TSValueRef allocate() {
        TSValueRef newObj = TSValueNullRef;
        if (firstFreeIdx_ < values_.size()) {
            newObj = (TSValueRef) firstFreeIdx_;
            firstFreeIdx_++;
        }
        return newObj;
    }
    
    ///
    void free(TSValueRef ref) {
        assert(validReference(ref));
        
        int swapIdx = firstFreeIdx_ - 1;
        values_[ref] = values_[swapIdx];
        firstFreeIdx_--;
    }
    
    ///
    T & get(TSValueRef ref) {
        assert(validReference(ref));
        return values_[ref];
    }
    
    ///
    bool validReference(TSValueRef ref) {
        return ref != TSValueNullRef
            && ref >= 0 && ref < firstFreeIdx_;
    }
    
    ///
    typedef typename std::vector<T>::iterator iterator;
    ///
    typedef typename std::vector<T>::const_iterator const_iterator;
    
    ///
    iterator begin() {
        return values_.begin();
    }
    
    ///
    const_iterator begin() const {
        return values_.begin();
    }
    
    ///
    iterator end() {
        return values_.end();
    }
    
    ///
    const_iterator end() const {
        return values_.end();
    }
};

#endif /* TSValuePool_hpp */
