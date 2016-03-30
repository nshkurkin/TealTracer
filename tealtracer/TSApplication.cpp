//
//  TSApplication.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TSApplication.hpp"

///
std::shared_ptr<TSApplication>
TSApplication::sharedApp_ = nullptr;

///
std::shared_ptr<TSApplication>
TSApplication::sharedApp() {
    return sharedApp_;
}

///
int
TSApplication::main(TSApplication * app, int argc, const char ** argv) {
    sharedApp_ = TSApplication::createManaged(app);
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(std::string(argv[i]));
    }
    return sharedApp_->run(args);
}

///
std::shared_ptr<TSWindow>
TSApplication::getWindow(int label) {
    return windows_[label];
}

///
void
TSApplication::createNewWindow(int label) {
    windows_[label] = newWindow();
}

///
void
TSApplication::removeWindow(int label) {
    windows_.erase(label);
}

///
TSApplication::window_itr
TSApplication::windowsBegin() {
    return windows_.begin();
}
///
TSApplication::window_itr
TSApplication::windowsEnd() {
    return windows_.end();
}
