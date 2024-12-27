//
// Created by eal on 12/27/24.
//

#include "MockUsageCollector.h"

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
    while (keepRunning_)
    {
        pulseStorage_->storePulse();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
