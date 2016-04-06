//
//  JobPool.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/5/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "JobPool.hpp"

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

void JobPool::emplaceJob(std::function<void(void)> dispatch,
 std::function<void(void)> callback) {
    pendingJobs.push_back(std::make_pair(dispatch, callback));
}

void JobPool::checkAndUpdateFinishedJobs() {
    std::vector<int> completed;
#ifndef __SINGLETHREADED__
    /// Push jobs onto the job wait pool if there is room available.
    while ((int) jobWaitPool.size() < maxNumThreads && pendingJobs.size() > 0) {
        jobWaitPool.emplace_back(std::async(
            std::launch::async,
            async_OGSDispatch,
            pendingJobs[0].first, pendingJobs[0].second
        ));
        pendingJobs.erase(pendingJobs.begin() + 0);
    }
    /// Check to see if any jobs completed
    for (int i = 0; i < (int) jobWaitPool.size(); i++) {
        std::future< std::function<void(void)> > & ftr = jobWaitPool[i];
        if (ftr.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            completed.push_back(i);
    }
#else
    int which = -1;
    while (++which < maxNumThreads && pendingJobs.size() > 0) {
        async_OGSDispatch(pendingJobs[0].first, pendingJobs[0].second);
        completed.push_back(which);
    }
#endif
    /// Cleanup any completed jobs
    for (int i = 0; i < (int) completed.size(); i++) {
        int index = completed[i] - i;
        std::function<void(void)> func = jobWaitPool[index].get();
        
        func();
        jobWaitPool.erase(jobWaitPool.begin() + index);
    }
}
