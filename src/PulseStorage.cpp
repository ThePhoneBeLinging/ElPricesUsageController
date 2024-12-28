//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"

PulseStorage::PulseStorage() : timeBetweenLastTwoPulses_(0) , lastPulseTime_(std::chrono::system_clock::now())
{
}

void PulseStorage::storePulse()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    double deltaTime = std::chrono::duration<double>(now - lastPulseTime_).count();
    timeBetweenLastTwoPulses_ = deltaTime;
    std::lock_guard lockGuard(mutex_);
    lastPulseTime_ = now;
}

double PulseStorage::getTimeBetweenPulses()
{
    std::lock_guard lockGuard(mutex_);
    return timeBetweenLastTwoPulses_;
}