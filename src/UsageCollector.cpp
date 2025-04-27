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
    std::thread thread;
    std::string chipPath;
    chipPath = ConfigController::getConfigString("ChipName");


    gpiod_chip *chip = gpiod_chip_open_by_name(chipPath.c_str());
    if (!chip) {
        throw std::runtime_error("Failed to open GPIO chip");
    }

    gpiod_line *in_line = gpiod_chip_get_line(chip, ConfigController::getConfigInt("InputPin"));
    if (!in_line || gpiod_line_request_input(in_line, "example") < 0) {
        throw std::runtime_error("Failed to request input line");
        gpiod_chip_close(chip);
    }

    gpiod_line* out_line = gpiod_chip_get_line(chip, ConfigController::getConfigInt("OutputPinForTest"));
    if (ConfigController::getConfigBool("UseMockPulse"))
    {
        if (!out_line || gpiod_line_request_output(out_line, "example",0) < 0) {
            throw std::runtime_error("Failed to request output line");
            gpiod_chip_close(chip);
        }
        thread = std::thread([this, out_line]()
        {
            int pulseLength = ConfigController::getConfigInt("MockPulseLengthInMS");
            int targetKWH = ConfigController::getConfigInt("TargetKWHForMockPulse");
            int delay = 360000 / targetKWH;
            while (keepRunning_)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                gpiod_line_set_value(out_line,1);
                std::this_thread::sleep_for(std::chrono::milliseconds(pulseLength));
                gpiod_line_set_value(out_line,0);
            }
        });
    }

    bool pulseActive = false;
    while (keepRunning_)
    {
        int sleepTime = ConfigController::getConfigInt("SleepTimeForPulseLoopInMS");
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));

        int val = gpiod_line_get_value(in_line);
        if (val == 1 && not pulseActive)
        {
            pulseActive = true;
            pulseStorage_->storePulse();
        }
        else if (val == 0 && pulseActive)
        {
            pulseActive = false;
        }
    }
    thread.join();
    gpiod_line_release(in_line);
    gpiod_line_release(out_line);
    gpiod_chip_close(chip);
}
