#ifndef GEO_NETWORK_CLIENT_AUDITHANDLER_H
#define GEO_NETWORK_CLIENT_AUDITHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/Types.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../crypto/lamportscheme.h"
#include "record/audit/AuditRecord.h"

#include "../../../libs/sqlite3/sqlite3.h"

using namespace crypto;

class AuditHandler {

public:
    AuditHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveAudit(
        AuditNumber number,
        TrustLineID trustLineID,
        lamport::KeyHash::Shared ownKeyHash,
        lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared contractorKeyHash,
        lamport::Signature::Shared contractorSignature,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &balance);

    const AuditRecord::Shared getActualAudit(
        TrustLineID trustLineID);

    const AuditRecord::Shared getActualAuditFull(
        TrustLineID trustLineID);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_AUDITHANDLER_H
