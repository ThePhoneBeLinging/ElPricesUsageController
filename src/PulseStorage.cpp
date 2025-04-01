//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"
#include <fmt/format.h>
#include <iostream>

#include "../../DatabaseAccessController/src/include/DatabaseAccessController/DatabaseAccessController.h"
#include "Utility/ConfigController.h"
#include "Utility/DebugController.h"
#include "Utility/TimeUtil.h"

PulseStorage::PulseStorage(const std::function<void(int pulsesCurrentHour, double currentWattage)>& onPulseFunction) : db_(std::make_unique<SQLite::Database>("../../HistoricData/Pulses.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
                               , keepRunning_(true),memoryDB_(std::make_unique<SQLite::Database>(":memory:",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
                               , lastPing_(std::chrono::high_resolution_clock::now()), wattageLast2Pulses_(0)
{
    onPulseFunction_ = onPulseFunction;
    std::cout << "PulseStorage constructor" << std::endl;
    DatabaseAccessController::addDatabase(db_,"PULSEDB");
    DatabaseAccessController::addDatabase(memoryDB_, "MEMORYPULSEDB");

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
    onPulseFunction_(pulsesCurrentHour_,wattageLast2Pulses_);

    auto memoryPulseDBLock = DatabaseAccessController::getDatabase("MEMORYPULSEDB");
    try
    {
        SQLite::Statement insertStatement(*memoryPulseDBLock->getDatabase(),"INSERT INTO Pulses DEFAULT VALUES");
        insertStatement.exec();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

int PulseStorage::getPulsesLastSeconds(int amountOfSeconds)
{
    auto memoryPulseDBLock = DatabaseAccessController::getDatabase("MEMORYPULSEDB");
    try
    {
        std::string query = "SELECT COUNT(*) FROM Pulses WHERE TimeStamp >= STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW', '-' || " + std::to_string(amountOfSeconds) + " || ' seconds')";
        int result = memoryPulseDBLock->getDatabase()->execAndGet(query).getInt();
        return result;
    }
    catch (const std::exception& e)
    {
        std::string debugText = e.what();
        DebugController::debugWrite("getPulsesLastSeconds: " + debugText);
    }

    return -1;
}

double PulseStorage::getWattage() const
{
    return wattageLast2Pulses_;
}

int PulseStorage::getPulsesLastHour()
{
    return pulsesCurrentHour_;
}

std::vector<std::shared_ptr<UsageDay>> PulseStorage::getUsageDays() const
{
    auto pulseDBLock = DatabaseAccessController::getDatabase("PULSEDB");

    try
    {
        std::vector<std::shared_ptr<UsageDay>> usageDays;
        SQLite::Statement selectUsageDays(*pulseDBLock->getDatabase(),"SELECT * FROM PulseDates");
        while (selectUsageDays.executeStep())
        {
            int id = selectUsageDays.getColumn(0).getInt();
            int year = selectUsageDays.getColumn(1).getInt();
            int month = selectUsageDays.getColumn(2).getInt();
            int day = selectUsageDays.getColumn(3).getInt();
            std::string usagedayString = fmt::format("{}-{}-{}",year,month,day);
            auto usageDay = std::make_shared<UsageDay>(usagedayString);

            SQLite::Statement selectPulseHours(*pulseDBLock->getDatabase(),"SELECT * FROM PulseHours WHERE PulseDateID == ?");
            selectPulseHours.bind(1,id);

            while (selectPulseHours.executeStep())
            {
                int hour = selectPulseHours.getColumn(1);
                int pulses = selectPulseHours.getColumn(2);
                usageDay->addHour(hour,pulses);
            }
            usageDays.push_back(usageDay);
        }
        return usageDays;
    }
    catch (const std::exception& exception)
    {
        std::string debugText = exception.what();
        DebugController::debugWrite("getUsageDays: " + debugText);
    }
    return {};
}

void PulseStorage::keepFileDBUpToDate()
{
    std::unique_lock<std::mutex> lock(condVarMutex_);
    int lastSavedHour = -1;
    int idToSaveHourUnder = 0;
    bool firstRun = true;
    while (keepRunning_)
    {
        if (not firstRun)
        {
            keepRunningCondition_.wait_for(lock,std::chrono::seconds(TimeUtil::secondsToNextHour()));
        }
        auto now = TimeUtil::getCurrentTime();
        if (lastSavedHour == now.tm_hour)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        auto pulseDBLock = DatabaseAccessController::getDatabase("PULSEDB");
        if (lastSavedHour == 23 || lastSavedHour == -1)
        {
            SQLite::Statement insertDate(*pulseDBLock->getDatabase(),"INSERT OR IGNORE INTO PulseDates(Year,Month,Day) VALUES(?,?,?)");
            insertDate.bind(1,now.tm_year);
            insertDate.bind(2,now.tm_mon);
            insertDate.bind(3,now.tm_mday);
            insertDate.exec();

            SQLite::Statement selectJustInserted(*pulseDBLock->getDatabase(),"SELECT ID FROM PulseDates WHERE Year==? AND Month==? AND Day==?");
            selectJustInserted.bind(1,now.tm_year);
            selectJustInserted.bind(2,now.tm_mon);
            selectJustInserted.bind(3,now.tm_mday);
            selectJustInserted.executeStep();
            idToSaveHourUnder = selectJustInserted.getColumn(0).getInt();
        }

        try
        {
            SQLite::Statement createHourLine(*pulseDBLock->getDatabase(),"INSERT OR IGNORE INTO PulseHours(PulseDateID,Hour,Pulses) VALUES(?,?,?)");
            createHourLine.bind(1,idToSaveHourUnder);
            createHourLine.bind(2,now.tm_hour);
            createHourLine.bind(3,0);
            createHourLine.exec();
            SQLite::Statement selectStatement(*pulseDBLock->getDatabase(),"SELECT Pulses FROM PulseHours WHERE PulseDateID == ? AND Hour == ?");
            selectStatement.bind(1,idToSaveHourUnder);
            selectStatement.bind(2,now.tm_hour);

            while (selectStatement.executeStep())
            {
                int pulses = selectStatement.getColumn(0).getInt();
                if (firstRun)
                {
                    pulsesCurrentHour_ = pulses;
                }
                else
                {
                    pulsesCurrentHour_ += pulses;
                }
                SQLite::Statement updatePulsesCurrentHourStatement(*pulseDBLock->getDatabase(),"UPDATE PulseHours SET Pulses = ? WHERE PulseDateID == ? AND Hour == ?");
                updatePulsesCurrentHourStatement.bind(1,pulsesCurrentHour_);
                updatePulsesCurrentHourStatement.bind(2,idToSaveHourUnder);
                updatePulsesCurrentHourStatement.bind(3,now.tm_hour);
                updatePulsesCurrentHourStatement.exec();
            }
            if (not firstRun)
            {
                pulsesCurrentHour_ = 0;
            }
        }
        catch (const std::exception& e)
        {
            std::string debugText = e.what();
            DebugController::debugWrite("KeepFileDBUpToDate " + debugText);
        }
        lastSavedHour = now.tm_hour;
        firstRun = false;
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
    auto memoryPulseDBLock = DatabaseAccessController::getDatabase("MEMORYPULSEDB");
    int amountOfSecondsToKeepInMemory = ConfigController::getConfigInt("ElPricesUsageControllerSecondsToKeepInMemory");
    try
    {
        std::string query = "SELECT * FROM Pulses WHERE TimeStamp < STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW', '-' || " + std::to_string(amountOfSecondsToKeepInMemory) + " || ' seconds')";
        SQLite::Statement queryStmt(*memoryPulseDBLock->getDatabase(), query);
        int pulses = 0;
        while (queryStmt.executeStep())
        {
            int id = queryStmt.getColumn(0).getInt();
            auto timeStamp = queryStmt.getColumn(1).getString();

            SQLite::Statement deleteCopiedRow(*memoryPulseDBLock->getDatabase(), "DELETE FROM Pulses WHERE ID = ?");
            deleteCopiedRow.bind(1, id);
            deleteCopiedRow.exec();
            pulses++;
        }
        DebugController::debugWrite("Deleted " + std::to_string(pulses) + " Pulses from memory DB");
    }
    catch (const std::exception& e)
    {
        std::string debugText = e.what();
        DebugController::debugWrite("CleanUpMemoryPulseDB(): " + debugText);
    }
}

