#include "OutgoingRemoteAddressNode.h"

OutgoingRemoteAddressNode::OutgoingRemoteAddressNode(
    BaseAddress::Shared address,
    UDPSocket &socket,
    IOService &ioService,
    Logger &logger):

    OutgoingRemoteBaseNode(
        socket,
        ioService,
        logger),
    mAddress(address)
{}

UDPEndpoint OutgoingRemoteAddressNode::remoteEndpoint() const
{
    return as::ip::udp::endpoint(
        as::ip::address_v4::from_string(
            mAddress->host()),
        mAddress->port());
}

string OutgoingRemoteAddressNode::remoteInfo() const
{
    return mAddress->fullAddress();
}

LoggerStream OutgoingRemoteAddressNode::errors() const
{
    return mLog.warning(
        string("Communicator / OutgoingRemoteAddressNode [")
        + remoteInfo()
        + string("]"));
}

LoggerStream OutgoingRemoteAddressNode::debug() const
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(
        string("Communicator / OutgoingRemoteAddressNode [")
        + remoteInfo()
        + string("]"));
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
