#ifndef OUTGOINGNODESHANDLER_H
#define OUTGOINGNODESHANDLER_H

#include "OutgoingRemoteBaseNode.h"
#include "../../../../providing/Provider.h"

#include <boost/unordered/unordered_map.hpp>
#include <forward_list>

using namespace std;


class OutgoingNodesHandler {
public:
    OutgoingNodesHandler (
        IOService &ioService,
        UDPSocket &socket,
        Logger &logger)
        noexcept;

    OutgoingRemoteBaseNode* handler(
        const IPv4WithPortAddress::Shared address)
        noexcept;

    OutgoingRemoteBaseNode* providerHandler(
        const IPv4WithPortAddress::Shared address)
        noexcept;

protected:
    static chrono::seconds kHandlersTTL()
        noexcept;

protected:
    void rescheduleCleaning()
        noexcept;

    void removeOutdatedNodeHandlers()
        noexcept;

    void removeOutdatedProviderHandlers()
        noexcept;

    static string logHeader()
        noexcept;

    LoggerStream debug() const
        noexcept;

protected:
    boost::unordered_map<string, OutgoingRemoteBaseNode::Unique> mNodes;
    boost::unordered_map<string, OutgoingRemoteBaseNode::Unique> mProviders;
    boost::unordered_map<string, DateTime> mLastAccessDateTimesNode;
    boost::unordered_map<string, DateTime> mLastAccessDateTimesProvider;

    boost::asio::steady_timer mCleaningTimer;

    IOService &mIOService;
    UDPSocket &mSocket;
    Logger &mLog;
};

#endif // OUTGOINGNODESHANDLER_H
