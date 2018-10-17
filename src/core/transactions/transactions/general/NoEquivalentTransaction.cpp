#include "NoEquivalentTransaction.h"

NoEquivalentTransaction::NoEquivalentTransaction(
    const NodeUUID &nodeUUID,
    BaseUserCommand::Shared command,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::NoEquivalentType,
        nodeUUID,
        0,
        logger),
    mCommand(command),
    mMessage(nullptr)
{}

NoEquivalentTransaction::NoEquivalentTransaction(
    const NodeUUID &nodeUUID,
    TransactionMessage::Shared message,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::NoEquivalentType,
        nodeUUID,
        0,
        logger),
    mCommand(nullptr),
    mMessage(message)
{}

TransactionResult::SharedConst NoEquivalentTransaction::run()
{
    if (mCommand != nullptr) {
        return transactionResultFromCommand(
            mCommand->responseEquivalentIsAbsent());
    } else if (mMessage != nullptr) {
        sendMessage<NoEquivalentMessage>(
            mMessage->senderUUID,
            mMessage->equivalent(),
            mNodeUUID,
            mMessage->transactionUUID());
    } else {
        error() << "Command and Message are empty";
    }
    return resultDone();
}

const string NoEquivalentTransaction::logHeader() const
{
    stringstream s;
    s << "[NoEquivalentTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}