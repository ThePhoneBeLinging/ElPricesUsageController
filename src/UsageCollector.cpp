//
// Created by eal on 10/03/25.
//

#include "UsageCollector.h"
#include <gpiod.h>
#include <iostream>

#include "Utility/ConfigController.h"

UsageCollector::UsageCollector(const std::shared_ptr<PulseStorage>& pulseStorage)
    : pulseStorage_(pulseStorage), keepRunning_(true)
{
    thread_ = std::thread(&UsageCollector::launchPulseThread, this);
}

UsageCollector::~UsageCollector()
{
    keepRunning_ = false;
    cv_.notify_one();
    thread_.join();
}

void UsageCollector::launchPulseThread()
{
    std::string chipPath;
    chipPath = ConfigController::getConfigString("ChipName");


    gpiod_chip *chip = gpiod_chip_open_by_name(chipPath.c_str());
    if (!chip) {
        std::cerr << "Failed to open GPIO chip\n";
    }

    gpiod_line *in_line = gpiod_chip_get_line(chip, ConfigController::getConfigInt("InputPin"));
    if (!in_line || gpiod_line_request_input(in_line, "example") < 0) {
        std::cerr << "Failed to request input line\n";
        gpiod_chip_close(chip);
    }

    gpiod_line_release(in_line);
    gpiod_chip_close(chip);
}
