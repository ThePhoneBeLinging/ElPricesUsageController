//
// Created by eal on 12/27/24.
//

#ifndef PULSESTORAGE_H
#define PULSESTORAGE_H
#include <memory>
#include "SQLiteCpp/Database.h"


class PulseStorage
{
public:
    PulseStorage();
    ~PulseStorage() = default;
    void storePulse();
    [[nodiscard]] double getTimeBetweenPulses();
private:
    std::unique_ptr<SQLite::Database> db_;
    std::unique_ptr<SQLite::Database> memoryDB_;

};



#endif //PULSESTORAGE_H
