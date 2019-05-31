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
        Logger &logger);

protected:
    UDPEndpoint remoteEndpoint() const override;

    string remoteInfo() const override;

    LoggerStream errors() const override;

    LoggerStream debug() const override;

protected:
    BaseAddress::Shared mAddress;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H
