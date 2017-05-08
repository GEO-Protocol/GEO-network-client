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
