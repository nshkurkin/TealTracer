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

struct JobPool {
private:
    std::vector< std::pair< std::function<void(void)>,
     std::function<void(void)> > > pendingJobs;
#ifndef __SINGLETHREADED__
    std::vector< std::future< std::function<void(void)> > > jobWaitPool;
#endif
public:
    int maxNumThreads;
    
    JobPool(int numThreads = 1);
    JobPool(const JobPool & other);
    JobPool & operator=(const JobPool & other);
    
    void emplaceJob(std::function<void(void)> dispatch, std::function<void(void)> callback);
    void checkAndUpdateFinishedJobs();

};

#endif
