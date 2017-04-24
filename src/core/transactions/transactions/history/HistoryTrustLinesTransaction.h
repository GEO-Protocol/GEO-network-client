#ifndef GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"
#include "../../../io/storage/HistoryStorage.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"

#include <vector>

class HistoryTrustLinesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryTrustLinesTransaction> Shared;

public:
    HistoryTrustLinesTransaction(
        NodeUUID &nodeUUID,
        HistoryTrustLinesCommand::Shared command,
        HistoryStorage *historyStorage,
        Logger *logger);

    HistoryTrustLinesCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        vector<pair<TrustLineRecord::Shared, DateTime>> trustLineRecords);

private:
    HistoryTrustLinesCommand::Shared mCommand;
    HistoryStorage *mHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
