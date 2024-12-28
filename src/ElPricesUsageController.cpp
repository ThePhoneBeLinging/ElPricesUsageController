//
// Created by Elias Aggergaard Larsen on 25/12/2024.
//

#include "ElPricesUsageController/ElPricesUsageController.h"

#include "MockUsageCollector.h"

ElPricesUsageController::ElPricesUsageController() : pulseStorage_(std::make_shared<PulseStorage>())
, usageCollector_(std::make_unique<MockUsageCollector>(pulseStorage_))
{

}

double ElPricesUsageController::getTimeBetweenPulses() const
{
    return pulseStorage_->getTimeBetweenPulses();
}
