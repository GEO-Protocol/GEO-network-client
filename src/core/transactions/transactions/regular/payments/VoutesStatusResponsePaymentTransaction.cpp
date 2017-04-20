#include "VoutesStatusResponsePaymentTransaction.h"

VoutesStatusResponsePaymentTransaction::VoutesStatusResponsePaymentTransaction(
        const NodeUUID &nodeUUID,
        VotesStatusRequestMessage::Shared message,
        PaymentOperationStateHandler *paymentOperationStateHandler,
        Logger *logger):
        BaseTransaction(
                BaseTransaction::TransactionType::VoutesStatusResponsePaymentTransaction,
                nodeUUID,
                logger),
        mPaymentOperationStateHandler(paymentOperationStateHandler),
        mRequestMessage(message),
        mLogger(logger)
{}

TransactionResult::SharedConst VoutesStatusResponsePaymentTransaction::run() {
    try {
        auto bufferAndSize = mPaymentOperationStateHandler->getState(mRequestMessage->transactionUUID());
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
