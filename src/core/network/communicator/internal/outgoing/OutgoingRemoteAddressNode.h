#ifndef GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H
#define GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H

#include "OutgoingRemoteBaseNode.h"

#include "../../../../contractors/addresses/BaseAddress.h"

class OutgoingRemoteAddressNode : public OutgoingRemoteBaseNode {
public:
    using Shared = shared_ptr<OutgoingRemoteAddressNode>;
    using Unique = unique_ptr<OutgoingRemoteAddressNode>;

public:
    OutgoingRemoteAddressNode(
        BaseAddress::Shared address,
        UDPSocket &socket,
        IOService &ioService,
        ContractorsManager *contractorsManager,
        Logger &logger)
        noexcept;

protected:
    void beginPacketsSending();

    LoggerStream errors() const;

    LoggerStream debug() const;

protected:
    BaseAddress::Shared mAddress;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H
