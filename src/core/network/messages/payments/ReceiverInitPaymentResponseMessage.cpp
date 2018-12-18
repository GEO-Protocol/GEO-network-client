#include "ReceiverInitPaymentResponseMessage.h"

ReceiverInitPaymentResponseMessage::ReceiverInitPaymentResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const OperationState state) :
    ResponseMessage(
        equivalent,
        senderUUID,
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
