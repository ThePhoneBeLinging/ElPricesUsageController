//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"

#include <iostream>

PulseStorage::PulseStorage() : memoryDB_(std::make_unique<SQLite::Database>(":memory:",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
{
    std::cout << "PulseStorage constructor" << std::endl;
    db_ = std::make_unique<SQLite::Database>("../../HistoricData/Pulses.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
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

void PulseStorage::memoryFlusherThreadFunction()
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    while (keepRunning_)
    {
        keepRunningCondition_.wait_for(lock,std::chrono::hours(1));
        dumpAllPulsesToFile();
    }
}

void PulseStorage::dumpAllPulsesToFile()
{
    std::lock_guard guard(databaseMutex_);
    // We keep the last 60 seconds of data. If i want to show more later, I will have to purge more data.
    int amountOfSeconds = 60;
    try
    {
        std::string query = "SELECT * FROM Pulses WHERE TimeStamp < STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW', '-' || " + std::to_string(amountOfSeconds) + " || ' seconds')";
        SQLite::Statement queryStmt(*memoryDB_, query);
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

        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

