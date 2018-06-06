#include "TransactionResult.h"

TransactionResult::TransactionResult()
{}

TransactionResult::TransactionResult(
    TransactionState::SharedConst transactionState)
{
    setTransactionState(transactionState);
}

TransactionResult::TransactionResult(
    CommandResult::SharedConst commandResult)
{
    setCommandResult(commandResult);
}

TransactionResult::TransactionResult(
    TransactionState::SharedConst transactionState,
    CommandResult::SharedConst commandResult)
{
    setTransactionState(transactionState);
    setCommandResult(commandResult);
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
    if (mCommandResult != nullptr && mTransactionState != nullptr) {
        return ResultType::HybridType;
    }

    else if (mCommandResult != nullptr) {
        return ResultType::CommandResultType;
    }

    return ResultType::TransactionStateType;
}