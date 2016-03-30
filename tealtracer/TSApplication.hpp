//
//  TSApplication.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSApplication_hpp
#define TSApplication_hpp

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "TSWindow.hpp"

///
class TSApplication : public TSManagedObject {
public:
    
    /// Call this once from `main` with a reference to the application you wish
    /// to start. Calls `run` on `app` with `argv` parsed into a std::string vector.
    static int main(TSApplication * app, int argc, const char ** argv);
    
    /// Access to the globally set application
    static std::shared_ptr<TSApplication> sharedApp();
    
    ///
    std::shared_ptr<TSWindow> getWindow(int label);
    ///
    void createNewWindow(int label);
    ///
    void removeWindow(int label);
    
    
    ///
    typedef std::map<int, std::shared_ptr<TSWindow>>::iterator window_itr;
    
    ///
    window_itr windowsBegin();
    ///
    window_itr windowsEnd();
    
    ///
    virtual void quit() = 0;
    
protected:
    
    /// Called when this object is set in `main`.
    virtual int run(const std::vector<std::string> & args) = 0;
    /// Called whenever a new window is going to be created.
    virtual std::shared_ptr<TSWindow> newWindow() = 0;

private:
    ///
    std::map<int, std::shared_ptr<TSWindow>> windows_;
    
    ///
    static std::shared_ptr<TSApplication> sharedApp_;
};

#endif /* TSApplication_hpp */
