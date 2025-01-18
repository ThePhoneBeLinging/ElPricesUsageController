//
// Created by eal on 12/27/24.
//

#ifndef PULSESTORAGE_H
#define PULSESTORAGE_H
#include <condition_variable>
#include <memory>
#include <thread>

#include "SQLiteCpp/Database.h"


class PulseStorage
{
public:
    PulseStorage();
    ~PulseStorage();
    void storePulse();
    int getPulsesLastSeconds(int amountOfSeconds);
    double getWattage() const;
private:
    void memoryFlusherThreadFunction();
    void dumpAllPulsesToFile(bool dumpAll);
    std::atomic_bool keepRunning_;
    std::condition_variable keepRunningCondition_;
    std::thread memoryFlusherThread_;
    std::mutex databaseMutex_;
    std::unique_ptr<SQLite::Database> db_;
    std::unique_ptr<SQLite::Database> memoryDB_;
    std::chrono::high_resolution_clock::time_point lastPing_;
    std::atomic<double> wattageLast2Pulses_;

};



#endif //PULSESTORAGE_H
