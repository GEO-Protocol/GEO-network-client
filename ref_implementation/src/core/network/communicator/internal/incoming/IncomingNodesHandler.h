/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_INCOMINGNODESHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGNODESHANDLER_H

#include "../common/Types.h"
#include "IncomingRemoteNode.h"
#include "MessageParser.h"

#include "../../../../common/time/TimeUtils.h"
#include "../../../../logger/Logger.h"

#include <boost/container/flat_map.hpp>

#include <forward_list>
#include <utility>


using namespace std;


class IncomingNodesHandler {
public:
    IncomingNodesHandler(
        MessagesParser &messagesParser,
        Logger &logger)
        noexcept;

    IncomingRemoteNode* handler(
        const UDPEndpoint &endpoint)
        noexcept;

    void removeOutdatedEndpoints();

    void removeOutdatedChannelsOfPresentEndpoints();

protected:
    static uint64_t key(
        const UDPEndpoint &endpoint)
        noexcept;

    LoggerStream debug() const
        noexcept;

protected:
    MessagesParser &mMessagesParser;
    Logger &mLog;

    boost::container::flat_map<uint64_t, IncomingRemoteNode::Unique> mNodes;
};


#endif //GEO_NETWORK_CLIENT_INCOMINGNODESHANDLER_H
