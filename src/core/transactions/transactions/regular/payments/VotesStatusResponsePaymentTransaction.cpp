#include "VotesStatusResponsePaymentTransaction.h"

VotesStatusResponsePaymentTransaction::VotesStatusResponsePaymentTransaction(
    const NodeUUID &nodeUUID,
    VotesStatusRequestMessage::Shared message,
    StorageHandler *storageHandler,
    bool isRequestedTransactionCurrentlyInProcessing,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::VoutesStatusResponsePaymentTransaction,
        nodeUUID,
        message->equivalent(),
        logger),
    mStorageHandler(storageHandler),
    mRequest(message),
    mIsRequestedTransactionCurrentlyInProcessing(isRequestedTransactionCurrentlyInProcessing)
{}

TransactionResult::SharedConst VotesStatusResponsePaymentTransaction::run()
{
    map<PaymentNodeID, lamport::Signature::Shared> emptySignatureMap;
    if (mIsRequestedTransactionCurrentlyInProcessing) {
        // if requested transaction didn't finish yet,
        // we send empty message, which means that requester should wait and ask again
        const auto kResponse = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            mNodeUUID,
            mRequest->transactionUUID(),
            emptySignatureMap);
        info() << "Requested TA currently is in processing. "
                "Send response with empty ParticipantsVotesMessage to "
                << mRequest->senderUUID;
        sendMessage(
            mRequest->senderUUID,
            kResponse);
        return resultDone();
    }
    try {
        // try to read participants vote message of requested transaction from database,
        // if it will be found then send it to requester
        auto ioTransaction = mStorageHandler->beginTransaction();
        auto serializedVotesBufferAndSize = ioTransaction->paymentOperationStateHandler()->byTransaction(
            mRequest->transactionUUID());
        const auto kResponse = make_shared<ParticipantsVotesMessage>(
            serializedVotesBufferAndSize.first);
        info() << "send response with not empty ParticipantsVotesMessage to " << mRequest->senderUUID;
        sendMessage(
            mRequest->senderUUID,
            kResponse);

    } catch(NotFoundError &) {
        const auto kResponse = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            mNodeUUID,
            mRequest->transactionUUID(),
            emptySignatureMap);
        info() << "Node don't know about requested TA. Send response with empty ParticipantsVotesMessage to "
               << mRequest->senderUUID;
        sendMessage(
            mRequest->senderUUID,
            kResponse);
    }
    return resultDone();
}

const string VotesStatusResponsePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[VotesStatusResponsePaymentTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
