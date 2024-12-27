//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ElPricesUsageController_H
#define ElPricesUsageController_H

#include <atomic>
#include <condition_variable>
#include <thread>



class ElPricesUsageController
{
public:
    ElPricesUsageController();
    ~ElPricesUsageController();


private:
    void keepUpdated();
    std::atomic<bool> keepRunningBool_;
    std::thread updatingThread_;
    std::condition_variable conditionVariable_;
};



#endif //ElPricesUsageController_H
