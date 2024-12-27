//
// Created by eal on 12/27/24.
//

#ifndef USAGECOLLECTOR_H
#define USAGECOLLECTOR_H
#include <atomic>
#include <thread>


class IUsageCollector
{
public:
    virtual ~IUsageCollector() = default;
    virtual void launchPulseThread() = 0;
    virtual int getPulseCount() = 0;
};

#endif //USAGECOLLECTOR_H
