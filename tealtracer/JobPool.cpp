//
//  JobPool.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/5/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "JobPool.hpp"
#include "TSLogger.hpp"

JobPool::JobPool(int numThreads) {
    maxNumThreads = numThreads;
}

JobPool::JobPool(const JobPool & other) {
    maxNumThreads = other.maxNumThreads;
}

JobPool & JobPool::operator=(const JobPool & other) {
    this->maxNumThreads = other.maxNumThreads;
    return *this;
}

static std::function<void(void)> async_OGSDispatch(
 std::function<void(void)> toRun, std::function<void(void)> toRet) {
    toRun();
    return toRet;
}

void JobPool::emplaceJob(const JobPool::WorkItem & workItem) {
    pendingJobs.push_back(workItem);
}

void JobPool::checkAndUpdateFinishedJobs() {
    std::vector<int> completed;
//    TSLoggerLog(std::cout, "checking jobs in pool");
#ifndef __SINGLETHREADED__
    
    /// Push jobs onto the job wait pool if there is room available.
    while ((int) jobWaitPool.size() < maxNumThreads && pendingJobs.size() > 0) {
        auto & workItem = *pendingJobs.begin();
        jobWaitPool.push_back(*pendingJobs.begin());
        jobWaitPool[jobWaitPool.size() - 1].workReturn = std::async(
            async_OGSDispatch,
            workItem.work, workItem.callback
        );

        pendingJobs.erase(pendingJobs.begin() + 0);
    }
    /// Check to see if any jobs completed
    for (int i = 0; i < (int) jobWaitPool.size(); i++) {
        std::future< std::function<void(void)> > & ftr = jobWaitPool[i].workReturn;
        if (ftr.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            completed.push_back(i);
        }
    }
#else
    int which = -1;
    while (++which < maxNumThreads && pendingJobs.size() > 0) {
        async_OGSDispatch(pendingJobs[0].work, pendingJobs[0].callback);
        completed.push_back(which);
    }
#endif
    /// Cleanup any completed jobs
    for (int i = 0; i < (int) completed.size(); i++) {
        int index = completed[i] - i;
        std::function<void(void)> func = jobWaitPool[index].workReturn.get();
        
        func();
        jobWaitPool.erase(jobWaitPool.begin() + index);
    }
    
//    TSLoggerLog(std::cout, "Done checking jobs in pool");
}
