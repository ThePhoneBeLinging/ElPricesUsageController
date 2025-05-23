//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#ifndef ElPricesUsageController_H
#define ElPricesUsageController_H

#include <condition_variable>
#include <thread>

#include "../../IUsageCollector.h"
#include "../../PulseStorage.h"


class ElPricesUsageController
{
public:
    explicit ElPricesUsageController(const std::function<void(int pulsesCurrentHour, double currentWattage)>& onPulseFunction);
    ~ElPricesUsageController() = default;
    int getAmountOfPulsesBasedOnSeconds(int seconds);
    [[nodiscard]] double getWattage() const;
    [[nodiscard]] int getPulsesLastHour() const;
    std::vector<std::shared_ptr<UsageDay>> getUsageDays() const;
private:
    std::shared_ptr<PulseStorage> pulseStorage_;
    std::unique_ptr<IUsageCollector> usageCollector_;
};



#endif //ElPricesUsageController_H
