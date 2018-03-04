#include "PaymentTransactionByCommandUUIDTransaction.h"

PaymentTransactionByCommandUUIDTransaction::PaymentTransactionByCommandUUIDTransaction(
    NodeUUID &nodeUUID,
    PaymentTransactionByCommandUUIDCommand::Shared command,
    BaseTransaction::Shared requestedTransaction,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::TransactionByCommandUUIDType,
        nodeUUID,
        0,      // none equivalent
        logger),
    mCommand(command),
    mRequestedPaymentTransaction(requestedTransaction)
{}

PaymentTransactionByCommandUUIDCommand::Shared PaymentTransactionByCommandUUIDTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst PaymentTransactionByCommandUUIDTransaction::run()
{
    stringstream stream;
    if (mRequestedPaymentTransaction == nullptr) {
        stream << "0";
        info() << "Requested transaction not found";
    } else {
        stream << "1" << kTokensSeparator
               << mRequestedPaymentTransaction->currentTransactionUUID().stringUUID();
        info() << "Requested transaction found";
    }
    auto result = stream.str();
    return resultOk(result);
}

TransactionResult::SharedConst PaymentTransactionByCommandUUIDTransaction::resultOk(
    string &transactionUUIDStr)
{
    return transactionResultFromCommand(
        mCommand->resultOk(
            transactionUUIDStr));
}

const string PaymentTransactionByCommandUUIDTransaction::logHeader() const
{
    stringstream s;
    s << "[PaymentTransactionByCommandUUIDTA: " << currentTransactionUUID() << "]";
    return s.str();
}
