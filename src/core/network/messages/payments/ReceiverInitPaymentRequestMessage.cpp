#include "ReceiverInitPaymentRequestMessage.h"

ReceiverInitPaymentRequestMessage::ReceiverInitPaymentRequestMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount) :
    RequestMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        0,
        amount)
{}

ReceiverInitPaymentRequestMessage::ReceiverInitPaymentRequestMessage(
    BytesShared buffer) :
    RequestMessage(
        buffer)
{}

const Message::MessageType ReceiverInitPaymentRequestMessage::typeID() const
{
    return Message::Payments_ReceiverInitPaymentRequest;
}
