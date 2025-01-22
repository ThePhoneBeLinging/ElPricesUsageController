//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"

#include <iostream>

#include "Utility/ConfigController.h"
#include "Utility/TimeUtil.h"

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
    fileDBWriterThread_ = std::thread(&PulseStorage::keepFileDBUpToDate,this);
}

PulseStorage::~PulseStorage()
{
    keepRunning_ = false;
    keepRunningCondition_.notify_all();
    memoryFlusherThread_.join();
    fileDBWriterThread_.join();
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
    pulsesCurrentHour_ += 1;

    std::lock_guard guard(memoryDatabaseMutex_);
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
    std::lock_guard guard(memoryDatabaseMutex_);
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

void PulseStorage::keepFileDBUpToDate()
{
    std::unique_lock<std::mutex> lock(condVarMutex_);
    int lastSavedHour = -1;
    int idToSaveHourUnder = 0;
    while (keepRunning_)
    {
        keepRunningCondition_.wait_for(lock,std::chrono::seconds(TimeUtil::secondsToNextHour()));
        auto now = TimeUtil::getCurrentTime();
        if (lastSavedHour == now.tm_hour)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        if (lastSavedHour == 23 || lastSavedHour == -1)
        {
            SQLite::Statement insertDate(*db_,"INSERT OR IGNORE INTO PulseDates(Year,Month,Day) VALUES(?,?,?)");
            insertDate.bind(1,now.tm_year);
            insertDate.bind(2,now.tm_mon);
            insertDate.bind(3,now.tm_mday);
            insertDate.exec();

            SQLite::Statement selectJustInserted(*db_,"SELECT ID FROM PulseDates WHERE Year==? AND Month==? AND Day==?");
            selectJustInserted.bind(1,now.tm_year);
            selectJustInserted.bind(2,now.tm_mon);
            selectJustInserted.bind(3,now.tm_mday);
            selectJustInserted.executeStep();
            idToSaveHourUnder = selectJustInserted.getColumn(0).getInt();
        }

        try
        {
            SQLite::Statement createHourLine(*db_,"INSERT OR IGNORE INTO PulseHours(PulseDateID,Hour,Pulses) VALUES(?,?,?)");
            createHourLine.bind(1,idToSaveHourUnder);
            createHourLine.bind(2,now.tm_hour);
            createHourLine.bind(3,0);
            createHourLine.exec();
            SQLite::Statement selectStatement(*db_,"SELECT Pulses FROM PulseHours WHERE PulseDateID == ? AND Hour == ?");
            selectStatement.bind(1,idToSaveHourUnder);
            selectStatement.bind(2,now.tm_hour);

            while (selectStatement.executeStep())
            {
                int pulses = selectStatement.getColumn(0).getInt();

                pulsesCurrentHour_ += pulses;
                SQLite::Statement updatePulsesCurrentHourStatement(*db_,"UPDATE PulseHours SET Pulses = ? WHERE PulseDateID == ? AND Hour == ?");
                updatePulsesCurrentHourStatement.bind(1,pulsesCurrentHour_);
                updatePulsesCurrentHourStatement.bind(2,idToSaveHourUnder);
                updatePulsesCurrentHourStatement.bind(3,now.tm_hour);
                updatePulsesCurrentHourStatement.exec();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "KEepFileDBUpToDate(): " << e.what() << std::endl;
        }

        lastSavedHour = now.tm_hour;
    }
}

void PulseStorage::memoryFlusherThreadFunction()
{
    std::unique_lock<std::mutex> lock(condVarMutex_);
    while (keepRunning_)
    {
        int secondsToWait = ConfigController::getConfigInt("ElPricesUsageControllerSecondsDumpDelay");
        keepRunningCondition_.wait_for(lock,std::chrono::seconds(secondsToWait));
        cleanUpMemoryPulseDB();
    }
}

void PulseStorage::cleanUpMemoryPulseDB()
{
    std::lock_guard guard(memoryDatabaseMutex_);
    int amountOfSecondsToKeepInMemory = ConfigController::getConfigInt("ElPricesUsageControllerSecondsToKeepInMemory");
    try
    {
        std::string query = "SELECT * FROM Pulses WHERE TimeStamp < STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW', '-' || " + std::to_string(amountOfSecondsToKeepInMemory) + " || ' seconds')";
        SQLite::Statement queryStmt(*memoryDB_, query);
        int pulses = 0;
        while (queryStmt.executeStep())
        {
            int id = queryStmt.getColumn(0).getInt();
            auto timeStamp = queryStmt.getColumn(1).getString();

            SQLite::Statement deleteCopiedRow(*memoryDB_, "DELETE FROM Pulses WHERE ID = ?");
            deleteCopiedRow.bind(1, id);
            deleteCopiedRow.exec();
            pulses++;
        }

        std::cout << "Deleted " + std::to_string(pulses) + " Pulses from memory DB\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

