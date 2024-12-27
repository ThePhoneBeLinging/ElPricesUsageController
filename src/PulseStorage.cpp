//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"

PulseStorage::PulseStorage()
{
    pulseTimes_.resize(maxPulsesPerInterval_);
    for (int i = 0; i < maxPulsesPerInterval_; i++)
    {
        pulseTimes_[i] = std::chrono::system_clock::now();
    }
}

void PulseStorage::storePulse()
{
    std::lock_guard lockGuard(mutex_);
    pulseTimes_[activeIndex_++] = std::chrono::system_clock::now();
    activeIndex_ %= maxPulsesPerInterval_;
}

int PulseStorage::getPulseCount()
{
    auto nowWithInterval = std::chrono::system_clock::now();

    auto interval = std::chrono::seconds(60);
    nowWithInterval -= interval;
    std::lock_guard lockGuard(mutex_);
    int amountOfPulses = 0;
    for (int i = 0; i < maxPulsesPerInterval_; i++)
    {
        if (nowWithInterval <= pulseTimes_[i])
        {
            amountOfPulses++;
        }
    }
    return amountOfPulses;
}
