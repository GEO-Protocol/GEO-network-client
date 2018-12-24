#include "ReceiverInitPaymentResponseMessage.h"

ReceiverInitPaymentResponseMessage::ReceiverInitPaymentResponseMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const OperationState state) :
    ResponseMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        0,
        state)
{}

ReceiverInitPaymentResponseMessage::ReceiverInitPaymentResponseMessage(
    BytesShared buffer) :
    ResponseMessage(
        buffer)
{}

const Message::MessageType ReceiverInitPaymentResponseMessage::typeID() const
{
    return Message::Payments_ReceiverInitPaymentResponse;
}
