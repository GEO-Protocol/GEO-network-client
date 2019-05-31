#include "OutgoingRemoteProvider.h"

OutgoingRemoteProvider::OutgoingRemoteProvider(
    Provider::Shared provider,
    UDPSocket &socket,
    IOService &ioService,
    Logger &logger):

    OutgoingRemoteBaseNode(
        socket,
        ioService,
        logger),
    mProvider(provider)
{}

UDPEndpoint OutgoingRemoteProvider::remoteEndpoint() const
{
    return as::ip::udp::endpoint(
        as::ip::address_v4::from_string(
            mProvider->mainAddress()->host()),
        mProvider->mainAddress()->port());
}

string OutgoingRemoteProvider::remoteInfo() const
{
    return mProvider->name();
}

LoggerStream OutgoingRemoteProvider::errors() const
{
    return mLog.warning(
        string("Communicator / OutgoingRemoteProvider [")
        + remoteInfo()
        + string("]"));
}

LoggerStream OutgoingRemoteProvider::debug() const
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(
        string("Communicator / OutgoingRemoteProvider [")
        + remoteInfo()
        + string("]"));
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}