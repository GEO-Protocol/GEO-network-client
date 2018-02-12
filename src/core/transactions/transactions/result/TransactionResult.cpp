#include "TransactionResult.h"

TransactionResult::TransactionResult()
{}

TransactionResult::TransactionResult(
    TransactionState::SharedConst transactionState)
{
    setTransactionState(transactionState);
}

void TransactionResult::setCommandResult(
    CommandResult::SharedConst commandResult)
{
    mCommandResult = commandResult;
}

void TransactionResult::setTransactionState(
    TransactionState::SharedConst transactionState)
{
    mTransactionState = transactionState;
}

CommandResult::SharedConst TransactionResult::commandResult() const
{
    return mCommandResult;
}

TransactionState::SharedConst TransactionResult::state() const
{
    return mTransactionState;
}

TransactionResult::ResultType TransactionResult::resultType() const
{
    if (mCommandResult != nullptr) {
        return ResultType::CommandResultType;
    }

    return ResultType::TransactionStateType;
}