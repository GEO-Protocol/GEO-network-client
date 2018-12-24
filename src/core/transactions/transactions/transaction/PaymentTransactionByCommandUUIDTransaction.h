#ifndef GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDTRANSACTION_H
#define GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/transactions/PaymentTransactionByCommandUUIDCommand.h"

class PaymentTransactionByCommandUUIDTransaction : public BaseTransaction {

public:
    typedef shared_ptr<PaymentTransactionByCommandUUIDTransaction> Shared;

public:
    PaymentTransactionByCommandUUIDTransaction(
        PaymentTransactionByCommandUUIDCommand::Shared command,
        BaseTransaction::Shared requestedTransaction,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        string &transactionUUIDStr);

private:
    PaymentTransactionByCommandUUIDCommand::Shared mCommand;
    BaseTransaction::Shared mRequestedPaymentTransaction;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDTRANSACTION_H
