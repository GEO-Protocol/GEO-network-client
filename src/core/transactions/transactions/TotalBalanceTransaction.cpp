#include "TotalBalanceTransaction.h"

TotalBalanceTransaction::TotalBalanceTransaction(
        TotalBalanceCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::TotalBalanceTransaction),
        mCommand(command) {}

TotalBalanceCommand::Shared TotalBalanceTransaction::command() const {
    return mCommand;
}

void TotalBalanceTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> TotalBalanceTransaction::run() {
    return make_pair(CommandResult::SharedConst(mCommand.get()->unexpectedErrorResult()),
                     TransactionState::SharedConst(new TransactionState(0)));
}
