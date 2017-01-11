#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    OpenTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::OpenTrustLineTransaction, scheduler),
    mCommand(command),
    mTrustLinesInterface(interface) {}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {
    return mCommand;
}

void OpenTrustLineTransaction::setContext(
    Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> OpenTrustLineTransaction::serializeContext() {}

pair<CommandResult::SharedConst, TransactionState::SharedConst> OpenTrustLineTransaction::run() {

    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->type()) {

            case BaseTransaction::TransactionType::OpenTrustLineTransaction: {
                OpenTrustLineTransaction::Shared openTrustLineTransaction = static_pointer_cast<OpenTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == openTrustLineTransaction->command()->contractorUUID()) {
                    return conflictErrorResult();
                }
                break;
            }

            case BaseTransaction::TransactionType::AcceptTrustLineTransaction: {
                //TODO:: create update trust line transaction class
                break;
            }

            case BaseTransaction::TransactionType::UpdateTrustLineTransaction: {
                UpdateTrustLineTransaction::Shared updateTrustLineTransaction = static_pointer_cast<UpdateTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == updateTrustLineTransaction->command()->contractorUUID()) {
                    return conflictErrorResult();
                }
                break;
            }

            case BaseTransaction::TransactionType::SetTrustLineTransaction: {
                //TODO:: create set trust line transaction class
                break;
            }

            case BaseTransaction::TransactionType::CloseTrustLineTransaction: {
                CloseTrustLineTransaction::Shared closeTrustLineTransaction = static_pointer_cast<CloseTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == closeTrustLineTransaction->command()->contractorUUID()) {
                    return conflictErrorResult();
                }
                break;
            }

            case BaseTransaction::TransactionType::RejectTrustLineTransaction: {
                //TODO:: create reject trust line transaction class
            }

            default: {
                break;
            }

        }
    }

    try {
        mTrustLinesInterface->open(
            mCommand->contractorUUID(),
            mCommand->amount()
        );

    } catch (ConflictError &e) {
        return trustLinePresentResult();
    }

    return resultOk();

}

pair<CommandResult::SharedConst, TransactionState::SharedConst> OpenTrustLineTransaction::resultOk() {

    return make_pair(
        CommandResult::SharedConst(mCommand.get()->resultOk()),
        nullptr);
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> OpenTrustLineTransaction::conflictErrorResult() {

    return make_pair(
        CommandResult::SharedConst(mCommand.get()->resultConflict()),
        nullptr);
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> OpenTrustLineTransaction::trustLinePresentResult() {

    return make_pair(
        CommandResult::SharedConst(mCommand.get()->trustLineAlreadyPresentResult()),
        nullptr);
}

