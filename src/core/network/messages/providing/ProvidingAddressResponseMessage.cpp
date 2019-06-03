#include "ProvidingAddressResponseMessage.h"

ProvidingAddressResponseMessage::ProvidingAddressResponseMessage(
    BytesShared buffer):
    mBuffer(buffer)
{}

const Message::MessageType ProvidingAddressResponseMessage::typeID() const 
{
    return Message::ProvidingAddressResponse;
}

BytesShared ProvidingAddressResponseMessage::buffer() const 
{
    return mBuffer;
}