//
// Created by eal on 10/03/25.
//

#ifndef IUSAGECOLLECTOR_H
#define IUSAGECOLLECTOR_H
#include "IUsageCollector.h"
#include "PulseStorage.h"


class UsageCollector : public IUsageCollector
{
public:
    explicit UsageCollector(const std::shared_ptr<PulseStorage>& pulseStorage);
    ~UsageCollector() override;
    void launchPulseThread() override;
private:
    std::shared_ptr<PulseStorage> pulseStorage_;
    std::thread thread_;
    std::condition_variable cv_;
    std::atomic<bool> keepRunning_;

};



#endif //IUSAGECOLLECTOR_H
