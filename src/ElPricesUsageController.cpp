//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesUsageController/ElPricesUsageController.h"

#include "MockUsageCollector.h"
#include "UsageCollector.h"
#include "Utility/ConfigController.h"

ElPricesUsageController::ElPricesUsageController() : pulseStorage_(std::make_shared<PulseStorage>())
{
    if (ConfigController::getConfigBool("RunningOnPI"))
    {
        usageCollector_ = std::make_unique<UsageCollector>(pulseStorage_);
    }
    else
    {
        usageCollector_ = std::make_unique<MockUsageCollector>(pulseStorage_);
    }
}

int ElPricesUsageController::getAmountOfPulsesBasedOnSeconds(int seconds)
{
    return pulseStorage_->getPulsesLastSeconds(seconds);
}

double ElPricesUsageController::getWattage() const
{
    return pulseStorage_->getWattage();
}

int ElPricesUsageController::getPulsesLastHour() const
{
    return pulseStorage_->getPulsesLastHour();
}

std::vector<std::shared_ptr<UsageDay>> ElPricesUsageController::getUsageDays() const
{
    return pulseStorage_->getUsageDays();
}
