#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseTrustLineCommand::Shared command,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::CloseTrustLineTransactionType,
        nodeUUID
    ),
    mCommand(command),
    mTrustLinesManager(manager) {

}

CloseTrustLineTransaction::CloseTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::CloseTrustLineTransactionType
    ),
    mTrustLinesManager(manager) {

    deserializeFromBytes(buffer);
}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> CloseTrustLineTransaction::serializeToBytes() const{

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

void CloseTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(buffer);
    BytesShared commandBufferShared = tryCalloc(CloseTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        commandBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        CloseTrustLineCommand::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mCommand = CloseTrustLineCommand::Shared(
        new CloseTrustLineCommand(
            commandBufferShared
        )
    );

}

TransactionResult::SharedConst CloseTrustLineTransaction::run() {

    try {
        switch (mStep) {

            case 1: {
                if (!isTransactionToContractorUnique()) {
                    return conflictErrorResult();
                }
                increaseStepsCounter();
            }

            case 2: {
                if (!isOutgoingTrustLineDirectionExisting()) {
                    return trustLineAbsentResult();
                }
                increaseStepsCounter();
            }

            case 3: {
                if (checkDebt()) {
                    suspendTrustLineDirectionToContractor();

                } else {
                    closeTrustLine();
                }
                increaseStepsCounter();
            }

        case 4: {
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
                throw ConflictError("CloseTrustLineTransaction::run: "
                                        "Illegal step execution.");
            }

        }

    } catch (exception &e) {
        throw RuntimeError("CloseTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + e.what());
    }
}

bool CloseTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool CloseTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
           mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

bool CloseTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mCommand->contractorUUID()) == BalanceRange::Positive;
}

void CloseTrustLineTransaction::suspendTrustLineDirectionToContractor() {

    mTrustLinesManager->suspendDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing
    );
}

void CloseTrustLineTransaction::closeTrustLine() {

    mTrustLinesManager->close(mCommand->contractorUUID());
}

TransactionResult::SharedConst CloseTrustLineTransaction::checkTransactionContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        auto responseMessage = mContext[kResponsePosition];
        if (responseMessage->typeID() == Message::MessageTypeID::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(responseMessage);
            switch (response->code()) {

                case RejectTrustLineMessage::kResultCodeRejected: {
                    return resultOk();
                }

                case RejectTrustLineMessage::kResultCodeRejectDelayed: {
                    return resultOk();
                }

                case RejectTrustLineMessage::kResultCodeTransactionConflict: {
                    return transactionConflictResult();
                }

                default: {
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

TransactionResult::SharedConst CloseTrustLineTransaction::waitingForResponseState() {

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

TransactionResult::SharedConst CloseTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst CloseTrustLineTransaction::trustLineAbsentResult() {

    return transactionResultFromCommand(mCommand->trustLineIsAbsentResult());
}

TransactionResult::SharedConst CloseTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(mCommand->resultConflict());
}

TransactionResult::SharedConst CloseTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(mCommand->resultNoResponse());
}

TransactionResult::SharedConst CloseTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(mCommand->resultTransactionConflict());
}

TransactionResult::SharedConst CloseTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(mCommand->unexpectedErrorResult());
}