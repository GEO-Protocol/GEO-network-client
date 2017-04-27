#include "VoutesStatusResponsePaymentTransaction.h"

VoutesStatusResponsePaymentTransaction::VoutesStatusResponsePaymentTransaction(
        const NodeUUID &nodeUUID,
        VotesStatusRequestMessage::Shared message,
        StorageHandler *storageHandler,
        Logger *logger):
        BaseTransaction(
                BaseTransaction::TransactionType::VoutesStatusResponsePaymentTransaction,
                nodeUUID,
                logger),
        mStorageHandler(storageHandler),
        mRequestMessage(message)
{}

TransactionResult::SharedConst VoutesStatusResponsePaymentTransaction::run() {
    try {
        auto ioTransation = mStorageHandler->beginTransaction();
        auto bufferAndSize = ioTransation->paymentOperationStateHandler()->getState(mRequestMessage->transactionUUID());
        auto responseMessage = make_shared<ParticipantsVotesMessage>(
                bufferAndSize.first
        );
    } catch(NotFoundError){
        const auto kZeroUUID = NodeUUID::empty();
        auto responseMessage = make_shared<ParticipantsVotesMessage>(
                mNodeUUID,
                mRequestMessage->transactionUUID(),
                kZeroUUID
        );
    sendMessage(
            mRequestMessage->senderUUID,
            responseMessage);
    };
    return resultDone();
}
