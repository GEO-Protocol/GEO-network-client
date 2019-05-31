#ifndef GEO_NETWORK_CLIENT_OUTGOINGREMOTEPROVIDER_H
#define GEO_NETWORK_CLIENT_OUTGOINGREMOTEPROVIDER_H

#include "OutgoingRemoteBaseNode.h"
#include "../../../../providing/Provider.h"

class OutgoingRemoteProvider : public OutgoingRemoteBaseNode {

public:
    using Unique = unique_ptr<OutgoingRemoteProvider>;

public:
    OutgoingRemoteProvider(
        Provider::Shared provider,
        UDPSocket &socket,
        IOService &ioService,
        Logger &logger);

protected:
    UDPEndpoint remoteEndpoint() const override;

    string remoteInfo() const override;

    LoggerStream errors() const override;

    LoggerStream debug() const override;

protected:
    Provider::Shared mProvider;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTEPROVIDER_H
