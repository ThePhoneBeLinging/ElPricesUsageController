//
// Created by eal on 12/27/24.
//

#ifndef PULSESTORAGE_H
#define PULSESTORAGE_H
#include <condition_variable>
#include <memory>
#include <thread>

#include "ElPricesUsageController/UsageDay.h"
#include "SQLiteCpp/Database.h"


class PulseStorage
{
public:
    explicit PulseStorage(const std::function<void(int pulsesCurrentHour, double currentWattage)>& onPulseFunction);
    ~PulseStorage();
    void storePulse();
    int getPulsesLastSeconds(int amountOfSeconds);
    double getWattage() const;
    int getPulsesLastHour();
    std::vector<std::shared_ptr<UsageDay>> getUsageDays() const;
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

    std::shared_ptr<SQLite::Database> db_;
    std::shared_ptr<SQLite::Database> memoryDB_;
    std::chrono::high_resolution_clock::time_point lastPing_;
    std::atomic<double> wattageLast2Pulses_;
    std::atomic<int> pulsesCurrentHour_;

    std::function<void(int pulsesCurrentHour, double currentWattage)> onPulseFunction_;

};



#endif //PULSESTORAGE_H
