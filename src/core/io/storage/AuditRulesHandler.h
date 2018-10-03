#ifndef GEO_NETWORK_CLIENT_AUDITRULESHANDLER_H
#define GEO_NETWORK_CLIENT_AUDITRULESHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/Types.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../trust_lines/audit_rules/BaseAuditRule.h"

#include "../../../libs/sqlite3/sqlite3.h"

class AuditRulesHandler {

public:
    AuditRulesHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveRule(
        TrustLineID trustLineID,
        BaseAuditRule::AuditRuleType auditRuleType);

    const BaseAuditRule::AuditRuleType getRule(
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


#endif //GEO_NETWORK_CLIENT_AUDITRULESHANDLER_H
