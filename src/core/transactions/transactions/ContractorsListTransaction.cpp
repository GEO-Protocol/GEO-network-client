#include "ContractorsListTransaction.h"

ContractorsListTransaction::ContractorsListTransaction(
        ContractorsListCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::ContractorsListTransaction),
        mCommand(command) {}

ContractorsListCommand::Shared ContractorsListTransaction::command() const {
    return mCommand;
}

void ContractorsListTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> ContractorsListTransaction::run() {
    return make_pair(CommandResult::SharedConst(mCommand.get()->unexpectedErrorResult()),
                     TransactionState::SharedConst(new TransactionState(0)));
}
