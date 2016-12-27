#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
        UpdateTrustLineCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::UpdateTrustLineTransaction),
        mCommand(command) {}

UpdateTrustLineCommand::Shared UpdateTrustLineTransaction::command() const {
    return mCommand;
}

void UpdateTrustLineTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> UpdateTrustLineTransaction::run() {
    return make_pair(CommandResult::SharedConst(nullptr),
                     TransactionState::SharedConst(new TransactionState(15000)));
}

pair<byte *, size_t> UpdateTrustLineTransaction::serializeContext() {

}
