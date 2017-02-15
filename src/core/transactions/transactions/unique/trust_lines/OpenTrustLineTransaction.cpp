#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    NodeUUID &nodeUUID,
    OpenTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::OpenTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mCommand(command),
    mTrustLinesManager(manager) {}

OpenTrustLineTransaction::OpenTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> OpenTrustLineTransaction::serializeToBytes() const {

    auto parentBytesAndCount = TrustLineTransaction::serializeToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +  commandBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        commandBytesAndCount.first.get(),
        commandBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void OpenTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(buffer);
    BytesShared commandBufferShared = tryCalloc(OpenTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        commandBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        OpenTrustLineCommand::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mCommand = OpenTrustLineCommand::Shared(
        new OpenTrustLineCommand(
            commandBufferShared
        )
    );
}

TransactionResult::SharedConst OpenTrustLineTransaction::run() {

    switch (mStep) {

        case 1: {
            if (isTransactionToContractorUnique()) {
                return conflictErrorResult();
            }
            increaseStepsCounter();
        }

        case 2: {
            if (isOutgoingTrustLineDirectionExisting()) {
                return trustLinePresentResult();
            }
            increaseStepsCounter();
        }

        case 3: {
            if (!mContext.empty()) {
                return checkTransactionContext();

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
            throw ConflictError("OpenTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

bool OpenTrustLineTransaction::isTransactionToContractorUnique() {

    auto transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->transactionType()) {

            case BaseTransaction::TransactionType::OpenTrustLineTransactionType: {
                OpenTrustLineTransaction::Shared openTrustLineTransaction = static_pointer_cast<OpenTrustLineTransaction>(it.first);
                if (mTransactionUUID != it.first->UUID()) {
                    if (mCommand->contractorUUID() == openTrustLineTransaction->command()->contractorUUID()) {
                        return true;
                    }
                }
                break;
            }

            case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                SetTrustLineTransaction::Shared setTrustLineTransaction = static_pointer_cast<SetTrustLineTransaction>(it.first);
                if (mTransactionUUID != it.first->UUID()) {
                    if (mCommand->contractorUUID() == setTrustLineTransaction->command()->contractorUUID()) {
                        return true;
                    }
                }
                break;
            }

            case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                CloseTrustLineTransaction::Shared closeTrustLineTransaction = static_pointer_cast<CloseTrustLineTransaction>(it.first);
                if (mTransactionUUID != it.first->UUID()) {
                    if (mCommand->contractorUUID() == closeTrustLineTransaction->command()->contractorUUID()) {
                        return true;
                    }
                }
                break;
            }

            default: {
                break;
            }

        }

    }

    return false;
}

bool OpenTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
        mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

TransactionResult::SharedConst OpenTrustLineTransaction::checkTransactionContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        auto responseMessage = mContext[kResponsePosition];
        if (responseMessage->typeID() == Message::MessageTypeID::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(responseMessage);
            switch (response->code()) {

                case AcceptTrustLineMessage::kResultCodeAccepted: {
                    openTrustLine();
                    return resultOk();
                }

                case AcceptTrustLineMessage::kResultCodeConflict: {
                    return conflictErrorResult();
                }

                case AcceptTrustLineMessage::kResultCodeTransactionConflict: {
                    return transactionConflictResult();
                }

                default:{
                    return unexpectedErrorResult();
                }

            }
        }

        return unexpectedErrorResult();

    } else {
        throw ConflictError("OpenTrustLineTransaction::checkTransactionContext: "
                                "Transaction waiting responses count " + to_string(kResponsesCount) +
                                " has " + to_string(mContext.size())
        );
    }
}

void OpenTrustLineTransaction::sendMessageToRemoteNode() {

    Message *message = new OpenTrustLineMessage(
        mNodeUUID,
        mTransactionUUID,
        mCommand->amount()
    );

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );
}

TransactionResult::SharedConst OpenTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(kConnectionTimeout * 1000)
        ),
        Message::MessageTypeID::ResponseMessageType,
        false
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::SharedConst(transactionState));
    return TransactionResult::SharedConst(transactionResult);
}

void OpenTrustLineTransaction::openTrustLine() {

    try {
        mTrustLinesManager->open(
            mCommand->contractorUUID(),
            mCommand->amount()
        );

    } catch (std::exception &e) {
        throw Exception(e.what());
    }
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst OpenTrustLineTransaction::trustLinePresentResult() {

    return transactionResultFromCommand(mCommand->trustLineAlreadyPresentResult());
}

TransactionResult::SharedConst OpenTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(mCommand->resultConflict());
}

TransactionResult::SharedConst OpenTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(mCommand->resultNoResponse());
}

TransactionResult::SharedConst OpenTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(mCommand->resultTransactionConflict());
}

TransactionResult::SharedConst OpenTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(mCommand->unexpectedErrorResult());
}