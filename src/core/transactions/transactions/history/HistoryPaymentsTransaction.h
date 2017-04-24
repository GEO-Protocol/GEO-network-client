#ifndef GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryPaymentsCommand.h"
#include "../../../io/storage/HistoryStorage.h"
#include "../../../io/storage/record/payment/PaymentRecord.h"

#include <vector>

class HistoryPaymentsTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryPaymentsTransaction> Shared;

public:
    HistoryPaymentsTransaction(
        NodeUUID &nodeUUID,
        HistoryPaymentsCommand::Shared command,
        HistoryStorage *historyStorage,
        Logger *logger);

    HistoryPaymentsCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        vector<pair<PaymentRecord::Shared, DateTime>> paymentRecords);

private:
    HistoryPaymentsCommand::Shared mCommand;
    HistoryStorage *mHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H
