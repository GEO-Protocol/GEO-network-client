#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    UpdateTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::UpdateTrustLineTransaction, scheduler),
    mCommand(command),
    mTrustLinesInterface(interface) {}

UpdateTrustLineCommand::Shared UpdateTrustLineTransaction::command() const {
    return mCommand;
}

void UpdateTrustLineTransaction::setContext(
    Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> UpdateTrustLineTransaction::serializeContext() {}

pair<CommandResult::SharedConst, TransactionState::SharedConst> UpdateTrustLineTransaction::run() {}

pair<CommandResult::SharedConst, TransactionState::SharedConst> UpdateTrustLineTransaction::conflictErrorResult() {

    return make_pair(
        CommandResult::SharedConst(mCommand.get()->resultConflict()),
        nullptr);
}
