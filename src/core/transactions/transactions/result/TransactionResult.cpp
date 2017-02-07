#include "TransactionResult.h"

TransactionResult::TransactionResult() {}

void TransactionResult::setCommandResult(
    CommandResult::SharedConst commandResult){

    mCommandResult = commandResult;
}

void TransactionResult::setMessageResult(
    MessageResult::Shared messageResult) {

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
