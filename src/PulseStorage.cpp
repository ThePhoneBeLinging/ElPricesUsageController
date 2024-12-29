//
// Created by eal on 12/27/24.
//

#include "PulseStorage.h"

#include <iostream>

PulseStorage::PulseStorage() : db_(std::make_unique<SQLite::Database>("../../HistoricData/Pulses.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
                               , memoryDB_(std::make_unique<SQLite::Database>(":memory:",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE))
{
    // This part of the constructor creates a Table with the same specifications of the file-based DB
    std::string query = "SELECT sql FROM sqlite_master WHERE type='table' AND name='Pulses';";

    SQLite::Statement queryStmt(*db_, query);
    if (queryStmt.executeStep())
    {

        std::string createTableSQL = queryStmt.getColumn(0).getString();
        memoryDB_->exec(createTableSQL);
    }
}

void PulseStorage::storePulse()
{
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

double PulseStorage::getTimeBetweenPulses()
{
    return 0;
}