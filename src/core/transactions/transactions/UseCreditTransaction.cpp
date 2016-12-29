#include "UseCreditTransaction.h"

UseCreditTransaction::UseCreditTransaction(
        UseCreditCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::UseCreditTransaction),
        mCommand(command) {}

UseCreditCommand::Shared UseCreditTransaction::command() const {
    return mCommand;
}

void UseCreditTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> UseCreditTransaction::run() {
    return make_pair(CommandResult::SharedConst(mCommand.get()->unexpectedErrorResult()),
                     TransactionState::SharedConst(new TransactionState(0)));
}

pair<byte *, size_t> UseCreditTransaction::serializeContext() {

}
