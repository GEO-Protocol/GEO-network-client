#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
        OpenTrustLineCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::OpenTrustLineTransaction), mCommand(command) {}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {
    return mCommand;
}

void OpenTrustLineTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> OpenTrustLineTransaction::run() {
    return make_pair(CommandResult::SharedConst(mCommand.get()->resultOk()),
                     nullptr);
}

pair<byte *, size_t> OpenTrustLineTransaction::serializeContext() {}
