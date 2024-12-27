//
// Created by eal on 12/27/24.
//

#ifndef PULSESTORAGE_H
#define PULSESTORAGE_H
#include <mutex>
#include <vector>
#include "Utility/TimeUtil.h"

class PulseStorage
{
public:
    PulseStorage();
    ~PulseStorage() = default;
    void storePulse();
    int getPulseCount();
private:
    const int maxPulsesPerInterval_ = 1024;
    int activeIndex_ = 0;
    std::mutex mutex_;
    std::vector<std::chrono::system_clock::time_point> pulseTimes_;

};



#endif //PULSESTORAGE_H
