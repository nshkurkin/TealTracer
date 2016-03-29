//
//  TSManagedObject.hpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSManagedObject_hpp
#define TSManagedObject_hpp

#include <memory>

///
class TSManagedObject {
public:
    TSManagedObject();
    virtual ~TSManagedObject();
    
    ///
    void setReference(std::shared_ptr<TSManagedObject> reference);
    ///
    std::shared_ptr<TSManagedObject> sharedReference() const;

    ///
    template <class C>
    static std::shared_ptr<C> createManaged(C * obj) {
        auto shared = std::shared_ptr<C>(obj);
        obj->setReference(shared);
        return shared;
    }

private:
    ///
    std::weak_ptr<TSManagedObject> managedSelf_;
};

#endif /* TSManagedObject_hpp */
