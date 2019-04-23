#ifndef GEO_NETWORK_CLIENT_OUTGOINGREMOTENODE_H
#define GEO_NETWORK_CLIENT_OUTGOINGREMOTENODE_H

#include "OutgoingRemoteBaseNode.h"

#include "../../../../contractors/Contractor.h"

class OutgoingRemoteNode : public OutgoingRemoteBaseNode {
public:
    using Shared = shared_ptr<OutgoingRemoteNode>;
    using Unique = unique_ptr<OutgoingRemoteNode>;

public:
    OutgoingRemoteNode(
        Contractor::Shared remoteContractor,
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
    Contractor::Shared mRemoteContractor;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTENODE_H
