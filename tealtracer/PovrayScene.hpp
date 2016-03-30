//
//  PovrayScene.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PovrayScene_hpp
#define PovrayScene_hpp

#include <string>
#include <memory>
#include <vector>

///
class PovraySceneElement {
public:
    virtual ~PovraySceneElement() {}
    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) = 0;
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const = 0;
};

///
class PovrayScene {
public:

    ///
    void addElement(std::shared_ptr<PovraySceneElement> element) {
        elements_.push_back(element);
    }

private:

    ///
    std::vector<std::shared_ptr<PovraySceneElement>> elements_;
};

#endif /* PovrayScene_hpp */
