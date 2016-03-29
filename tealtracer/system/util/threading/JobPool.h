///
///
///
///
///
///

#ifndef ____util_JobPool__
#define ____util_JobPool__

#ifndef __SINGLETHREADED__
#include <future>
#include <thread>
#include <chrono>
#endif

#include <vector>
#include <functional>

namespace util {
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
        
        void emplaceJob(std::function<void(void)> dispatch,
         std::function<void(void)> callback);
        void checkAndUpdateFinishedJobs();
    
    };
}

#endif
