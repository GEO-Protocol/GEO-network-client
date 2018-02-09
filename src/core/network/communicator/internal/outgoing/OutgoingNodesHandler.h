#ifndef OUTGOINGNODESHANDLER_H
#define OUTGOINGNODESHANDLER_H

#include "OutgoingRemoteNode.h"

#include <boost/unordered/unordered_map.hpp>
#include <forward_list>

using namespace std;


class OutgoingNodesHandler {
public:
    OutgoingNodesHandler (
        IOService &ioService,
        UDPSocket &socket,
        UUID2Address &UUID2AddressService,
        Logger &logger)
        noexcept;

    OutgoingRemoteNode* handler(
        const NodeUUID &nodeUUID)
        noexcept;

protected:
    static chrono::seconds kHandlersTTL()
        noexcept;

protected:
    void rescheduleCleaning()
        noexcept;

    void removeOutdatedHandlers()
        noexcept;

    static string logHeader()
        noexcept;

    LoggerStream debug() const
        noexcept;

protected:
    boost::unordered_map<NodeUUID, OutgoingRemoteNode::Unique> mNodes;
    boost::unordered_map<NodeUUID, DateTime> mLastAccessDateTimes;
    boost::asio::steady_timer mCleaningTimer;

    IOService &mIOService;
    UDPSocket &mSocket;
    UUID2Address &mUUID2AddressService;
    Logger &mLog;
};

#endif // OUTGOINGNODESHANDLER_H
