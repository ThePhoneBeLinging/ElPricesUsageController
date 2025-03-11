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
    //chipPath = ConfigController::getConfigString("ChipName");
    chipPath = "gpiochip0";


    gpiod_chip *chip = gpiod_chip_open_by_name(chipPath.c_str());
    if (!chip) {
        std::cerr << "Failed to open GPIO chip\n";
    }

    // Set up output line
    gpiod_line *out_line = gpiod_chip_get_line(chip, ConfigController::getConfigInt("OutputPinForTest"));
    if (!out_line || gpiod_line_request_output(out_line, "example", 0) < 0) {
        std::cerr << "Failed to request output line\n";
        gpiod_chip_close(chip);
    }
    gpiod_line_set_value(out_line,1);

    // Set up input line
    gpiod_line *in_line = gpiod_chip_get_line(chip, ConfigController::getConfigInt("InputPin"));
    if (!in_line || gpiod_line_request_input(in_line, "example") < 0) {
        std::cerr << "Failed to request input line\n";
        gpiod_chip_close(chip);
    }
    std::atomic_bool localKeepRunning = true;
    std::condition_variable localCV;
    auto testLambda = [out_line, &localKeepRunning, &localCV] ()-> void
    {
        std::mutex mutex;
        std::unique_lock lock(mutex);
        while (localKeepRunning)
        {
            double targetKWH = ConfigController::getConfigInt("TargetKWHForMockPulse");
            int waitTime = 3600 / targetKWH;
            gpiod_line_set_value(out_line,1);
            localCV.wait_for(lock, std::chrono::milliseconds(waitTime));
            gpiod_line_set_value(out_line,1);
            int pulseLength = ConfigController::getConfigInt("MockPulseLengthInMS");
            std::this_thread::sleep_for(std::chrono::milliseconds(pulseLength));
        }
    };


    auto thread = std::thread(testLambda);
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
    localKeepRunning = false;
    localCV.notify_one();
    thread_.join();
    gpiod_line_release(out_line);
    gpiod_line_release(in_line);
    gpiod_chip_close(chip);
}
