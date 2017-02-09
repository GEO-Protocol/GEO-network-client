#include "TransactionResult.h"

TransactionResult::TransactionResult() {}

TransactionResult::TransactionResult(
    TransactionState::Shared transactionState) {

    setTransactionState(transactionState);
}

void TransactionResult::setCommandResult(
    CommandResult::SharedConst commandResult){

    mCommandResult = commandResult;
}

void TransactionResult::setMessageResult(
    MessageResult::SharedConst messageResult) {

    mMessageResult = messageResult;
}

void TransactionResult::setTransactionState(
    TransactionState::Shared transactionState) {

    mTransactionState = transactionState;
}

CommandResult::SharedConst TransactionResult::commandResult() const {

    return mCommandResult;
}

MessageResult::SharedConst TransactionResult::messageResult() const {

    return mMessageResult;
}

TransactionState::SharedConst TransactionResult::state() const {

    return mTransactionState;
}

TransactionResult::ResultType TransactionResult::resultType() const {

    if (mMessageResult.get() != nullptr) {
        return ResultType::MessageResultType;
    }

    return ResultType::CommandResultType;
}
