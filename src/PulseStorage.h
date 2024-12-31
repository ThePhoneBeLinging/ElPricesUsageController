//
// Created by eal on 12/27/24.
//

#ifndef PULSESTORAGE_H
#define PULSESTORAGE_H
#include <condition_variable>
#include <memory>
#include "SQLiteCpp/Database.h"


class PulseStorage
{
public:
    PulseStorage();
    ~PulseStorage();
    void storePulse();
    int getPulsesLastSeconds(int amountOfSeconds);
private:
    void memoryFlusherThreadFunction();
    void dumpAllPulsesToFile();
    std::atomic_bool keepRunning_;
    std::condition_variable keepRunningCondition_;
    std::thread memoryFlusherThread_;
    std::mutex databaseMutex_;
    std::unique_ptr<SQLite::Database> db_;
    std::unique_ptr<SQLite::Database> memoryDB_;

};



#endif //PULSESTORAGE_H
