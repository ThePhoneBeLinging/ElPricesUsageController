//
// Created by Elias Aggergaard Larsen on 24/01/2025.
//

#include "ElPricesUsageController/UsageDay.h"

UsageDay::UsageDay(std::string dateString) : dateString_(std::move(dateString))
{
    usageHours_.resize(24);
}

std::string UsageDay::getDateString()
{
    return dateString_;
}

void UsageDay::addHour(int hour, int pulses)
{
    usageHours_[hour] = pulses;
}

std::vector<int> UsageDay::getUsageHours()
{
    return usageHours_;
}
