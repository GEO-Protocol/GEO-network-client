#ifndef GEO_NETWORK_CLIENT_OUTGOINGREMOTENODE_H
#define GEO_NETWORK_CLIENT_OUTGOINGREMOTENODE_H

#include "OutgoingRemoteBaseNode.h"

#include "../../../../contractors/ContractorsManager.h"

class OutgoingRemoteNode : public OutgoingRemoteBaseNode {
public:
    using Shared = shared_ptr<OutgoingRemoteNode>;
    using Unique = unique_ptr<OutgoingRemoteNode>;

public:
    OutgoingRemoteNode(
        ContractorID remoteContractorID,
        UDPSocket &socket,
        IOService &ioService,
        ContractorsManager *contractorsManager,
        Logger &logger);

protected:
    UDPEndpoint remoteEndpoint() const override;

    string remoteInfo() const override;

    LoggerStream errors() const override;

    LoggerStream debug() const override;

protected:
    ContractorID mRemoteContractorID;
    ContractorsManager *mContractorsManager;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTENODE_H
