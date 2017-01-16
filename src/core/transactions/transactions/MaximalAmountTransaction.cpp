#include "MaximalAmountTransaction.h"

MaximalAmountTransaction::MaximalAmountTransaction(
        MaximalTransactionAmountCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::MaximalAmountTransactionType),
        mCommand(command) {}

MaximalTransactionAmountCommand::Shared MaximalAmountTransaction::command() const {
    return mCommand;
}

void MaximalAmountTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> MaximalAmountTransaction::serializeContext() {}

TransactionResult::Shared MaximalAmountTransaction::run() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(new TransactionState(10)));
    return TransactionResult::Shared(transactionResult);
}
