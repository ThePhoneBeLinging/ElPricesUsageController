//
// Created by eal on 12/27/24.
//

#include "MockUsageCollector.h"

#include <random>

MockUsageCollector::MockUsageCollector(const std::shared_ptr<PulseStorage>& pulseStorage) : pulseStorage_(pulseStorage), keepRunning_(true)
{
    thread_ = std::thread(&MockUsageCollector::launchPulseThread,this);
}

MockUsageCollector::~MockUsageCollector()
{
    keepRunning_ = false;
    thread_.join();
}

void MockUsageCollector::launchPulseThread()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> dist(50, 1000);
    while (keepRunning_)
    {
        pulseStorage_->storePulse();
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(eng)));
    }
}
