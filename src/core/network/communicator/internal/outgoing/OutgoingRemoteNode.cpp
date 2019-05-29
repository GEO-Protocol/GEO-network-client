#include "OutgoingRemoteNode.h"

OutgoingRemoteNode::OutgoingRemoteNode(
    ContractorID remoteContractorID,
    UDPSocket &socket,
    IOService &ioService,
    ContractorsManager *contractorsManager,
    Logger &logger):

    OutgoingRemoteBaseNode(
        socket,
        ioService,
        logger),
    mRemoteContractorID(remoteContractorID),
    mContractorsManager(contractorsManager)
{}

MsgEncryptor::Buffer OutgoingRemoteNode::preprocessMessage(
    Message::Shared message) const
{
    if(!message->isEncrypted()) {
        return message->serializeToBytes();
    }
    return MsgEncryptor(
        mContractorsManager->contractor(message->contractorId())->cryptoKey()->contractorPublicKey
    ).encrypt(message);
}

UDPEndpoint OutgoingRemoteNode::remoteEndpoint() const
{
    auto remoteContractor = mContractorsManager->contractor(
        mRemoteContractorID);
    return as::ip::udp::endpoint(
        as::ip::address_v4::from_string(
            remoteContractor->mainAddress()->host()),
        remoteContractor->mainAddress()->port());
}

string OutgoingRemoteNode::remoteInfo() const
{
    return to_string(mRemoteContractorID);
}

LoggerStream OutgoingRemoteNode::errors() const
{
    return mLog.warning(
        string("Communicator / OutgoingRemoteNode [")
        + remoteInfo()
        + string("]"));
}

LoggerStream OutgoingRemoteNode::debug() const
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(
        string("Communicator / OutgoingRemoteNodeNew [")
        + remoteInfo()
        + string("]"));
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
