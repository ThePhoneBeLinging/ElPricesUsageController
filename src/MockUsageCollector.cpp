//
// Created by eal on 12/27/24.
//

#include "MockUsageCollector.h"
#include <random>
#include <gpiod.h>
#include <iostream>

#include "Utility/ConfigController.h"

MockUsageCollector::MockUsageCollector(const std::shared_ptr<PulseStorage>& pulseStorage) : pulseStorage_(pulseStorage), keepRunning_(true)
{
    thread_ = std::thread(&MockUsageCollector::launchPulseThread,this);
}

MockUsageCollector::~MockUsageCollector()
{
    keepRunning_ = false;
    cv_.notify_all();
    thread_.join();
}

void MockUsageCollector::launchPulseThread()
{
    std::string chipPath;
    //chipPath = ConfigController::getConfigString("ChipName");
    chipPath = "gpiochip4";
    gpiod_chip *chip = gpiod_chip_open_by_name(chipPath.c_str());
    gpiod_line *out_line = gpiod_chip_get_line(chip, ConfigController::getConfigInt("OutputPinForTest"));
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> dist(1500, 15000);
    std::mutex mutex;
    std::unique_lock lock(mutex);
    while (keepRunning_)
    {
        if (not ConfigController::getConfigBool("RunningOnPI"))
        {
            pulseStorage_->storePulse();
            cv_.wait_for(lock,std::chrono::milliseconds(dist(eng)));
        }
        else
        {
            std::cout << "low" << std::endl;
            gpiod_line_set_value(out_line,0);
        }

        if (ConfigController::getConfigBool("RunningOnPI"))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            gpiod_line_set_value(out_line,1);
            std::cout << "HIGH" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(ConfigController::getConfigInt("MockPulseLengthInMS")));
        }
    }
}
