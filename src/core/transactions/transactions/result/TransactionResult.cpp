#include "TransactionResult.h"

TransactionResult::TransactionResult() {}

TransactionResult::TransactionResult(
    bool isFinishedSuccessful) {

    mFinishedWithoutResult = isFinishedSuccessful;
}

TransactionResult::TransactionResult(
    TransactionState::SharedConst transactionState) {

    setTransactionState(transactionState);
}

TransactionResult::SharedConst TransactionResult::finishSuccessfulWithoutResult() {

    return make_shared<const TransactionResult>(
        true
    );
}

void TransactionResult::setCommandResult(
    CommandResult::SharedConst commandResult) {

    mCommandResult = commandResult;
}

void TransactionResult::setMessageResult(
    MessageResult::SharedConst messageResult) {

    mMessageResult = messageResult;
}

void TransactionResult::setTransactionState(
    TransactionState::SharedConst transactionState) {

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

const bool TransactionResult::isFinishedSuccessfulWithoutResult() const {

    return mFinishedWithoutResult;
}

TransactionResult::ResultType TransactionResult::resultType() const {

    if (mCommandResult != nullptr) {
        if (mMessageResult != nullptr) {
            throw ConflictError(
                "TransactionResult::resultType: "
                    "transaction result could't have message command result and message result at the same time.");
        }
        return ResultType::CommandResultType;
    }

    if (mMessageResult != nullptr) {
        if (mCommandResult != nullptr) {
            throw ConflictError(
                "TransactionResult::resultType: "
                    "transaction result could't have message result and command result at the same time.");
        }
        return ResultType::MessageResultType;
    }

    return ResultType::TransactionStateType;
}