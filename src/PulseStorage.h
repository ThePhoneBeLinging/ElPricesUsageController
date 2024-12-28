//
// Created by eal on 12/27/24.
//

#ifndef PULSESTORAGE_H
#define PULSESTORAGE_H
#include <mutex>
#include <vector>

class PulseStorage
{
public:
    PulseStorage();
    ~PulseStorage() = default;
    void storePulse();
    [[nodiscard]] double getTimeBetweenPulses() const;
private:
    double timeBetweenLastTwoPulses_;
    std::chrono::high_resolution_clock::time_point lastPulseTime_;
    std::mutex mutex_;

};



#endif //PULSESTORAGE_H
