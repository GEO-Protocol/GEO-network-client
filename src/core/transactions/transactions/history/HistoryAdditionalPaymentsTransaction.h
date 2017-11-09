#ifndef GEO_NETWORK_CLIENT_HISTORYADDITIONALPAYMENTSTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYADDITIONALPAYMENTSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryAdditionalPaymentsCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/payment/PaymentRecord.h"

#include <vector>

class HistoryAdditionalPaymentsTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryAdditionalPaymentsTransaction> Shared;

public:
    HistoryAdditionalPaymentsTransaction(
        NodeUUID &nodeUUID,
        HistoryAdditionalPaymentsCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    HistoryAdditionalPaymentsCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        const vector<PaymentRecord::Shared> &records);

private:
    HistoryAdditionalPaymentsCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_HISTORYADDITIONALPAYMENTSTRANSACTION_H
