//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesUsageController/ElPricesUsageController.h"

ElPricesUsageController::ElPricesUsageController() : keepRunningBool_(true)
{
    updatingThread_ = std::thread(&ElPricesUsageController::keepUpdated,this);
}

ElPricesUsageController::~ElPricesUsageController()
{
    keepRunningBool_ = false;
    conditionVariable_.notify_all();
    updatingThread_.join();
}

void ElPricesUsageController::keepUpdated()
{
    std::mutex mutex_;
    std::unique_lock lock(mutex_);
    while (keepRunningBool_)
    {
        conditionVariable_.wait_for(lock, std::chrono::hours(1));
    }
}
