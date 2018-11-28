#ifndef GEO_NETWORK_CLIENT_CONTRACTORSHANDLER_H
#define GEO_NETWORK_CLIENT_CONTRACTORSHANDLER_H

#include "../../contractors/Contractor.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

class ContractorsHandler {

public:
    ContractorsHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveContractor(
        Contractor::Shared contractor);

    void saveContractorFull(
        Contractor::Shared contractor);

    void saveIdOnContractorSide(
        Contractor::Shared contractor);

    vector<Contractor::Shared> allContractors();

    vector<ContractorID> allIDs();

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORSHANDLER_H
