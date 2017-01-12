#include "MaximalAmountTransaction.h"

MaximalAmountTransaction::MaximalAmountTransaction(
        MaximalTransactionAmountCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::MaximalAmountTransactionType),
        mCommand(command) {}

MaximalTransactionAmountCommand::Shared MaximalAmountTransaction::command() const {
    return mCommand;
}

void MaximalAmountTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> MaximalAmountTransaction::run() {
    return make_pair(CommandResult::SharedConst(mCommand.get()->unexpectedErrorResult()),
                     TransactionState::SharedConst(new TransactionState(0)));
}

pair<byte *, size_t> MaximalAmountTransaction::serializeContext() {

}
