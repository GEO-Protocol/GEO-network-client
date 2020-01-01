#ifndef GEO_NETWORK_CLIENT_HISTORYPAYMENTSALLEQUIVALENTSTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYPAYMENTSALLEQUIVALENTSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryPaymentsAllEquivalentsCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/payment/PaymentRecord.h"

#include <vector>

class HistoryPaymentsAllEquivalentsTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryPaymentsAllEquivalentsTransaction> Shared;

public:
    HistoryPaymentsAllEquivalentsTransaction(
        HistoryPaymentsAllEquivalentsCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    TransactionResult::SharedConst resultOk(
        const vector<PaymentRecord::Shared> &records);

private:
    HistoryPaymentsAllEquivalentsCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_HISTORYPAYMENTSALLEQUIVALENTSTRANSACTION_H
