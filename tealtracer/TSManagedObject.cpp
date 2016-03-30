//
//  TSManagedObject.cpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TSManagedObject.hpp"

///
TSManagedObject::TSManagedObject() {}

///
TSManagedObject::~TSManagedObject() {
    /// Remove access to control block, thus freeing the control block
    managedSelf_.reset();
}

///
void
TSManagedObject::setReference(std::shared_ptr<TSManagedObject> reference) {
    managedSelf_ = reference;
}
