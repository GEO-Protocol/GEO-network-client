#include "ContractorsListTransaction.h"

ContractorsListTransaction::ContractorsListTransaction(
        ContractorsListCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::ContractorsListTransactionType),
        mCommand(command) {}

ContractorsListCommand::Shared ContractorsListTransaction::command() const {
    return mCommand;
}

void ContractorsListTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> ContractorsListTransaction::serializeContext() {}

TransactionResult::Shared ContractorsListTransaction::run() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(new TransactionState(10)));
    return TransactionResult::Shared(transactionResult);
}
