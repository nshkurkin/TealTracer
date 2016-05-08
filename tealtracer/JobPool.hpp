//
//  JobPool.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/5/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef JobPool_hpp
#define JobPool_hpp

#ifndef __SINGLETHREADED__
#include <future>
#include <thread>
#include <chrono>
#endif

#include <vector>
#include <functional>
#include <string>

struct JobPool {
public:

    struct WorkItem {
        std::function<void(void)> work, callback;
        std::string identifier;
        
        WorkItem() : identifier(""), work([](){}), callback([](){}) {}
        WorkItem(std::string id, std::function<void(void)> work, std::function<void(void)> callback) : identifier(id), work(work), callback(callback) {}
        WorkItem(const WorkItem & other) {
            work = other.work;
            callback = other.callback;
            identifier = other.identifier;
        }
        
        WorkItem & operator=(const WorkItem & other) {
            work = other.work;
            callback = other.callback;
            identifier = other.identifier;
            return *this;
        }
        
    protected:
    
        friend struct JobPool;
    #ifndef __SINGLETHREADED__
        std::future< std::function<void(void)> > workReturn;
    #endif
    };

private:
    std::vector<WorkItem> pendingJobs;
#ifndef __SINGLETHREADED__
    std::vector<WorkItem> jobWaitPool;
#endif
public:
    int maxNumThreads;
    
    JobPool(int numThreads = 1);
    JobPool(const JobPool & other);
    JobPool & operator=(const JobPool & other);
    
    void emplaceJob(const WorkItem & workItem);
    void checkAndUpdateFinishedJobs();

};

#endif
