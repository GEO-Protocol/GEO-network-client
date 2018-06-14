#include "VotesStatusResponsePaymentTransaction.h"

VotesStatusResponsePaymentTransaction::VotesStatusResponsePaymentTransaction(
    const NodeUUID &nodeUUID,
    VotesStatusRequestMessage::Shared message,
    StorageHandler *storageHandler,
    bool isRequestedTransactionCurrentlyRunned,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::VoutesStatusResponsePaymentTransaction,
        nodeUUID,
        message->equivalent(),
        logger),
    mStorageHandler(storageHandler),
    mRequest(message),
    mIsRequestedTransactionCurrentlyRunned(isRequestedTransactionCurrentlyRunned)
{}

// todo change this logik according to crypto changes
TransactionResult::SharedConst VotesStatusResponsePaymentTransaction::run()
{
    debug() << "run";
    map<PaymentNodeID, lamport::Signature::Shared> emptySignMap;
    if (mIsRequestedTransactionCurrentlyRunned) {
        // if requested transaction didn't finish yet,
        // we send empty message, which means that requester should wait and ask again
        const auto kResponse = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            mNodeUUID,
            mRequest->transactionUUID(),
            emptySignMap);
        debug() << "send response with empty ParticipantsVotesMessage to " << mRequest->senderUUID;
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
        debug() << "send response with not empty ParticipantsVotesMessage to " << mRequest->senderUUID;
        sendMessage(
            mRequest->senderUUID,
            kResponse);

    } catch(NotFoundError &) {
        // If node was offline and it does not have serialize VotesMessage.
        // it means that coordinator didn't accept this transaction (maybe crash)
        // in this case we send reject message
        const auto kResponse = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            mNodeUUID,
            mRequest->transactionUUID(),
            emptySignMap);
        debug() << "send reject response to " << mRequest->senderUUID;
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
