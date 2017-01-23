#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
    NodeUUID &nodeUUID,
    CloseTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
        BaseTransaction::TransactionType::CloseTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mCommand(command),
    mTrustLinesManager(manager){

}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {

    return mCommand;
}

TransactionResult::Shared CloseTrustLineTransaction::run() {

    switch(mStep) {

        case 1: {
            if (checkSameTypeTransactions()) {
                return conflictErrorResult();
            }
            increaseStepsCounter();
        }

        case 2: {
            if (!checkTrustLineExisting()) {
                return trustLineAbsentResult();
            }
            increaseStepsCounter();
        }

        case 3: {
            if (checkDebt()) {
                pendingSuspendTrustLineToContractor();

            } else {
                suspendTrustLineToContractor();
            }
            increaseStepsCounter();
        }

        case 4: {
            if (mContext.get() != nullptr) {
                //TODO:: check context
                return resultOk();

            } else {
                if (mRequestCounter < kMaxRequestsCount) {
                    sendMessageToRemoteNode();
                    increaseRequestsCounter();

                } else {
                    return noResponseResult();
                }
            }
            return waitingForResponseState();
        }

        default: {
            throw ConflictError("CloseTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }

}

bool CloseTrustLineTransaction::checkSameTypeTransactions() {

    auto transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->transactionType()) {

            case BaseTransaction::TransactionType::OpenTrustLineTransactionType: {
                OpenTrustLineTransaction::Shared openTrustLineTransaction = static_pointer_cast<OpenTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == openTrustLineTransaction->command()->contractorUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                break;
            }

            case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                break;
            }

            default: {
                break;
            }

        }

    }

    return false;
}

bool CloseTrustLineTransaction::checkTrustLineExisting() {

    return mTrustLinesManager->checkDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing
    );
}

bool CloseTrustLineTransaction::checkDebt() {

    auto trustLine = mTrustLinesManager->trustLineByContractorUUID(mCommand->contractorUUID());
    return trustLine->balanceRange() == BalanceRange::Positive;
}

void CloseTrustLineTransaction::pendingSuspendTrustLineToContractor() {

    auto trustLine = mTrustLinesManager->trustLineByContractorUUID(mCommand->contractorUUID());
    trustLine->pendingSuspendOutgoingDirection();
}

void CloseTrustLineTransaction::suspendTrustLineToContractor() {

    auto trustLine = mTrustLinesManager->trustLineByContractorUUID(mCommand->contractorUUID());
    trustLine->suspendOutgoingDirection();
}

void CloseTrustLineTransaction::sendMessageToRemoteNode() {

    Message *message = new CloseTrustLineMessage(
        mNodeUUID,
        mTransactionUUID,
        mNodeUUID
    );

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );
}

TransactionResult::Shared CloseTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        kConnectionTimeout,
        Message::MessageTypeID::ResponseMessageType
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared CloseTrustLineTransaction::resultOk() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultOk())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared CloseTrustLineTransaction::trustLineAbsentResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->trustLineIsAbsentResult())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared CloseTrustLineTransaction::conflictErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultConflict())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared CloseTrustLineTransaction::noResponseResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultNoResponse())));
    return TransactionResult::Shared(transactionResult);
}
