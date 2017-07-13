#include "VotesStatusResponsePaymentTransaction.h"

VotesStatusResponsePaymentTransaction::VotesStatusResponsePaymentTransaction(
    const NodeUUID &nodeUUID,
    VotesStatusRequestMessage::Shared message,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::VoutesStatusResponsePaymentTransaction,
        nodeUUID,
        logger),
    mStorageHandler(storageHandler),
    mRequest(message)
{}

TransactionResult::SharedConst VotesStatusResponsePaymentTransaction::run()
{
    debug() << "run";
    try {
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
        // So set CoordinatorUUID as empty
        const auto kZeroUUID = NodeUUID::empty();
        const auto kResponse = make_shared<ParticipantsVotesMessage>(
            mNodeUUID,
            mRequest->transactionUUID(),
            kZeroUUID);
        debug() << "send response with empty ParticipantsVotesMessage to " << mRequest->senderUUID;
        sendMessage(
            mRequest->senderUUID,
            kResponse);
    };
    return resultDone();
}

const string VotesStatusResponsePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[VotesStatusResponsePaymentTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
