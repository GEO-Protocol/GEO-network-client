/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
