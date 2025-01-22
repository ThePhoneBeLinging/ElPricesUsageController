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
    void keepFileDBUpToDate();
    void memoryFlusherThreadFunction();
    void cleanUpMemoryPulseDB();
    std::atomic_bool keepRunning_;
    std::condition_variable keepRunningCondition_;
    std::mutex condVarMutex_;
    std::thread memoryFlusherThread_;
    std::mutex memoryDatabaseMutex_;
    std::thread fileDBWriterThread_;

    std::unique_ptr<SQLite::Database> db_;
    std::unique_ptr<SQLite::Database> memoryDB_;
    std::chrono::high_resolution_clock::time_point lastPing_;
    std::atomic<double> wattageLast2Pulses_;
    std::atomic<int> pulsesCurrentHour_;

};



#endif //PULSESTORAGE_H
