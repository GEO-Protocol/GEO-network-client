#include "ConfirmChannelMessage.h"

ConfirmChannelMessage::ConfirmChannelMessage(
    const TransactionUUID &transactionUUID,
    Contractor::Shared contractor):

    TransactionMessage(
        0,
        contractor->ownIdOnContractorSide(),
        transactionUUID)
{
    encrypt(contractor);
}

ConfirmChannelMessage::ConfirmChannelMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{}

const Message::MessageType ConfirmChannelMessage::typeID() const
{
    return Message::Channel_Confirm;
}