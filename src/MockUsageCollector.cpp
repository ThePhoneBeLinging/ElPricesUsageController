//
// Created by eal on 12/27/24.
//

#include "MockUsageCollector.h"
#include <random>
#include <gpiod.h>
#include <iostream>

#include "Utility/ConfigController.h"

MockUsageCollector::MockUsageCollector(const std::shared_ptr<PulseStorage>& pulseStorage) : pulseStorage_(pulseStorage), keepRunning_(true)
{
    thread_ = std::thread(&MockUsageCollector::launchPulseThread,this);
}

MockUsageCollector::~MockUsageCollector()
{
    keepRunning_ = false;
    cv_.notify_all();
    thread_.join();
}

void MockUsageCollector::launchPulseThread()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> dist(1500, 15000);
    std::mutex mutex;
    std::unique_lock lock(mutex);
    while (keepRunning_)
    {
        pulseStorage_->storePulse();
        cv_.wait_for(lock,std::chrono::milliseconds(dist(eng)));
    }
}
