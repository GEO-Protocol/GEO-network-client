#include "PaymentTransactionByCommandUUIDTransaction.h"

PaymentTransactionByCommandUUIDTransaction::PaymentTransactionByCommandUUIDTransaction(
    NodeUUID &nodeUUID,
    PaymentTransactionByCommandUUIDCommand::Shared command,
    BaseTransaction::Shared reuqestedTransaction,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::TransactionByCommandUUIDType,
        nodeUUID,
        logger),
    mCommand(command),
    mRequestedPaymentTransaction(reuqestedTransaction)
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
        stream << "1" << BaseUserCommand::kTokensSeparator
               << mRequestedPaymentTransaction->currentTransactionUUID().stringUUID();
        info() << "Requested transaction found";
    }
    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(result));
}

const string PaymentTransactionByCommandUUIDTransaction::logHeader() const
{
    stringstream s;
    s << "[PaymentTransactionByCommandUUIDTA: " << currentTransactionUUID() << "]";
    return s.str();
}
