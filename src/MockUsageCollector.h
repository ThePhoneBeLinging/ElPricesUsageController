//
// Created by eal on 12/27/24.
//

#ifndef MOCKUSAGECOLLECTOR_H
#define MOCKUSAGECOLLECTOR_H
#include <memory>

#include "IUsageCollector.h"
#include "PulseStorage.h"


class MockUsageCollector : public IUsageCollector
{
public:
    MockUsageCollector(const std::shared_ptr<PulseStorage>& pulseStorage);
    virtual ~MockUsageCollector();
    void launchPulseThread() override;
private:
    std::shared_ptr<PulseStorage> pulseStorage_;
    std::thread thread_;
    std::atomic<bool> keepRunning_;
};



#endif //MOCKUSAGECOLLECTOR_H
