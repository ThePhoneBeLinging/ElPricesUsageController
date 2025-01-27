//
// Created by Elias Aggergaard Larsen on 24/01/2025.
//

#ifndef USAGEDAY_H
#define USAGEDAY_H
#include <string>
#include <utility>
#include <vector>


class UsageDay
{
public:
    explicit UsageDay(std::string dateString);
    std::string getDateString();
    void addHour(int hour, int pulses);
    std::vector<int> getUsageHours();
private:
    std::string dateString_;
    std::vector<int> usageHours_;
};



#endif //USAGEDAY_H
