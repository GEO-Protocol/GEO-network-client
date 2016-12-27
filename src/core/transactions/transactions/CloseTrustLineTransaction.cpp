#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
        CloseTrustLineCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::CloseTrustLineTransaction),
        mCommand(command) {}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {
    return mCommand;
}

void CloseTrustLineTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> CloseTrustLineTransaction::run() {
    throw ValueError("Can't close trust line");
    //return make_pair(CommandResult::SharedConst(mCommand.get()->resultOk()),
    //                 TransactionState::SharedConst(new TransactionState(0)));
}

pair<byte *, size_t> CloseTrustLineTransaction::serializeContext() {

}
