//
// Created by eal on 12/27/24.
//

#ifndef MOCKUSAGECOLLECTOR_H
#define MOCKUSAGECOLLECTOR_H
#include "IUsageCollector.h"


class MockUsageCollector : public IUsageCollector
{
public:
    MockUsageCollector();
    virtual ~MockUsageCollector();
    void launchPulseThread() override;
    int getPulseCount() override;
private:
};



#endif //MOCKUSAGECOLLECTOR_H
