#ifndef GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryPaymentsCommand.h"
#include "../../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../../db/operations_history_storage/record/payment/PaymentRecord.h"

#include <vector>

using namespace db::operations_history_storage;

class HistoryPaymentsTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryPaymentsTransaction> Shared;

public:
    HistoryPaymentsTransaction(
        NodeUUID &nodeUUID,
        HistoryPaymentsCommand::Shared command,
        OperationsHistoryStorage *historyStorage,
        Logger *logger);

    HistoryPaymentsCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    TransactionResult::SharedConst resultOk(
        vector<PaymentRecord::Shared> paymentRecords);

private:

    HistoryPaymentsCommand::Shared mCommand;
    OperationsHistoryStorage *mHistoryStorage;

};


#endif //GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H
