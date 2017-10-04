#ifndef GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"

#include <vector>

class HistoryTrustLinesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryTrustLinesTransaction> Shared;

public:
    HistoryTrustLinesTransaction(
        NodeUUID &nodeUUID,
        HistoryTrustLinesCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    HistoryTrustLinesCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        const vector<TrustLineRecord::Shared> &records);

private:
    HistoryTrustLinesCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
