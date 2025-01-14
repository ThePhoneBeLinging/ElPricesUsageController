//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesUsageController/ElPricesUsageController.h"

#include "MockUsageCollector.h"

ElPricesUsageController::ElPricesUsageController() : pulseStorage_(std::make_shared<PulseStorage>())
, usageCollector_(std::make_unique<MockUsageCollector>(pulseStorage_))
{

}

int ElPricesUsageController::getAmountOfPulsesBasedOnSeconds(int seconds)
{
    return pulseStorage_->getPulsesLastSeconds(seconds);
}

double ElPricesUsageController::getWattage() const
{
    return pulseStorage_->getWattage();
}
