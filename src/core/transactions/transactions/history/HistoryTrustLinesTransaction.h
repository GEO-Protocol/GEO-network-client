#ifndef GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"
#include "../../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../../db/operations_history_storage/record/trust_line/TrustLineRecord.h"

#include <vector>

using namespace db::operations_history_storage;

class HistoryTrustLinesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryTrustLinesTransaction> Shared;

public:
    HistoryTrustLinesTransaction(
        NodeUUID &nodeUUID,
        HistoryTrustLinesCommand::Shared command,
        OperationsHistoryStorage *historyStorage,
        Logger *logger);

    HistoryTrustLinesCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    TransactionResult::SharedConst resultOk(
        vector<TrustLineRecord::Shared> trustLineRecords);

private:

    HistoryTrustLinesCommand::Shared mCommand;
    OperationsHistoryStorage *mHistoryStorage;

};


#endif //GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
