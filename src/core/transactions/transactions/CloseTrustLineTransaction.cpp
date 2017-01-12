#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
    CloseTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::CloseTrustLineTransactionType, scheduler),
    mCommand(command),
    mTrustLinesInterface(interface) {}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {
    return mCommand;
}

void CloseTrustLineTransaction::setContext(
    Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> CloseTrustLineTransaction::serializeContext() {}

pair<CommandResult::SharedConst, TransactionState::SharedConst> CloseTrustLineTransaction::run() {}

pair<CommandResult::SharedConst, TransactionState::SharedConst> CloseTrustLineTransaction::conflictErrorResult() {

    return make_pair(
        CommandResult::SharedConst(mCommand.get()->resultConflict()),
        nullptr);
}

