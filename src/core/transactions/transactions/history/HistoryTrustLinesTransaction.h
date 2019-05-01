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
        HistoryTrustLinesCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    TransactionResult::SharedConst resultOk(
        const vector<TrustLineRecord::Shared> &records);

private:
    HistoryTrustLinesCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_HISTORYTRUSTLINESTRANSACTION_H
