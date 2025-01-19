//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"

#include <iostream>

#include "Utility/ConfigController.h"

PulseStorage::PulseStorage() : db_(std::make_unique<SQLite::Database>("../../HistoricData/Pulses.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
                               , keepRunning_(true),memoryDB_(std::make_unique<SQLite::Database>(":memory:",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
, lastPing_(std::chrono::high_resolution_clock::now()), wattageLast2Pulses_(0)
{
    std::cout << "PulseStorage constructor" << std::endl;
    // This part of the constructor creates a Table with the same specifications of the file-based DB
    std::string query = "CREATE TABLE Pulses ("
    "\"ID\" INTEGER PRIMARY KEY AUTOINCREMENT,"
    "\"TimeStamp\" DATETIME NOT NULL DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW'))"
    ")";

    SQLite::Statement queryStmt(*memoryDB_, query);
    queryStmt.exec();
    memoryFlusherThread_ = std::thread(&PulseStorage::memoryFlusherThreadFunction, this);
}

PulseStorage::~PulseStorage()
{
    keepRunning_ = false;
    keepRunningCondition_.notify_one();
    memoryFlusherThread_.join();
}

void PulseStorage::storePulse()
{
    double deltaTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - lastPing_).count();
    wattageLast2Pulses_ = 3.6 / deltaTime;
    if (wattageLast2Pulses_ > 1000)
    {
        wattageLast2Pulses_ = 0;
    }
    lastPing_ = std::chrono::high_resolution_clock::now();

    std::lock_guard guard(databaseMutex_);
    try
    {
        SQLite::Statement insertStatement(*memoryDB_,"INSERT INTO Pulses DEFAULT VALUES");
        insertStatement.exec();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

int PulseStorage::getPulsesLastSeconds(int amountOfSeconds)
{
    std::lock_guard guard(databaseMutex_);
    try
    {
        std::string query = "SELECT COUNT(*) FROM Pulses WHERE TimeStamp >= STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW', '-' || " + std::to_string(amountOfSeconds) + " || ' seconds')";
        return memoryDB_->execAndGet(query).getInt();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return -1;
}

double PulseStorage::getWattage() const
{
    return wattageLast2Pulses_;
}

void PulseStorage::memoryFlusherThreadFunction()
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    while (keepRunning_)
    {
        int secondsToWait = ConfigController::getConfigInt("ElPricesUsageControllerSecondsDumpDelay");
        keepRunningCondition_.wait_for(lock,std::chrono::seconds(secondsToWait));
        dumpAllPulsesToFile(false);
    }
    dumpAllPulsesToFile(true);
}

void PulseStorage::dumpAllPulsesToFile(bool dumpAll)
{
    std::lock_guard guard(databaseMutex_);

    int amountOfSecondsToKeepInMemory = ConfigController::getConfigInt("ElPricesUsageControllerSecondsToKeepInMemory");
    if (dumpAll)
    {
        amountOfSecondsToKeepInMemory = 0;
    }
    try
    {
        std::string query = "SELECT * FROM Pulses WHERE TimeStamp < STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW', '-' || " + std::to_string(amountOfSecondsToKeepInMemory) + " || ' seconds')";
        SQLite::Statement queryStmt(*memoryDB_, query);
        int pulses = 0;
        while (queryStmt.executeStep())
        {
            int id = queryStmt.getColumn(0).getInt();
            auto timeStamp = queryStmt.getColumn(1).getString();

            SQLite::Statement insertToFileBasedDB(*db_, "INSERT INTO Pulses(Timestamp) VALUES(?)");
            insertToFileBasedDB.bind(1, timeStamp);
            insertToFileBasedDB.exec();

            SQLite::Statement deleteCopiedRow(*memoryDB_, "DELETE FROM Pulses WHERE ID = ?");
            deleteCopiedRow.bind(1, id);
            deleteCopiedRow.exec();
            pulses++;
        }
        std::cout << "Saved " + std::to_string(pulses) + " Pulses to file DB\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

