#include "VotesStatusResponsePaymentTransaction.h"

VotesStatusResponsePaymentTransaction::VotesStatusResponsePaymentTransaction(
    const NodeUUID &nodeUUID,
    VotesStatusRequestMessage::Shared message,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    bool isRequestedTransactionCurrentlyInProcessing,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::VoutesStatusResponsePaymentTransaction,
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler),
    mRequest(message),
    mIsRequestedTransactionCurrentlyInProcessing(isRequestedTransactionCurrentlyInProcessing)
{}

TransactionResult::SharedConst VotesStatusResponsePaymentTransaction::run()
{
    auto senderAddress = mRequest->senderAddresses.at(0);
    info() << "Requested node " << senderAddress->fullAddress();
    map<PaymentNodeID, lamport::Signature::Shared> emptySignatureMap;
    if (mIsRequestedTransactionCurrentlyInProcessing) {
        // if requested transaction didn't finish yet,
        // we send empty message, which means that requester should wait and ask again
        info() << "Requested TA currently is in processing. "
                "Send response with empty ParticipantsVotesMessage to "
                << mRequest->senderUUID;
        sendMessage<ParticipantsVotesMessage>(
            senderAddress,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            mRequest->transactionUUID(),
            emptySignatureMap);
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto participantsSignatures = ioTransaction->paymentParticipantsVotesHandler()->participantsSignatures(
        mRequest->transactionUUID());
    if (!participantsSignatures.empty()) {
        // todo : check if requested node is participant of this transaction
        info() << "send response with not empty ParticipantsVotesMessage";
        sendMessage<ParticipantsVotesMessage>(
            senderAddress,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            mRequest->transactionUUID(),
            participantsSignatures);

    } else {
        info() << "Node don't know about requested TA. Send response with empty ParticipantsVotesMessage";
        sendMessage<ParticipantsVotesMessage>(
            senderAddress,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            mRequest->transactionUUID(),
            emptySignatureMap);
    }
    return resultDone();
}

const string VotesStatusResponsePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[VotesStatusResponsePaymentTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
