#ifndef GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H


#include "../../common/NodeUUID.h"
#include "../messages/Message.h"
#include "OutgoingMessagesQueue.h"

#include "vector"
#include "map"

class OutgoingMessagesHandler {
public:
    OutgoingMessagesHandler();

    void sendMessageToTheNode(const std::shared_ptr <NodeUUID> nodeUUID, const std::shared_ptr <Message> message);

private:
    bool containsQueueFor(const std::shared_ptr <NodeUUID> nodeUUID) const;

    void initQueueFor(const std::shared_ptr <NodeUUID> nodeUUID);

    OutgoingMessagesQueue *queueFor(const std::shared_ptr <NodeUUID> nodeUUID) const;

private:
    std::map<NodeUUID, OutgoingMessagesQueue *> *mOutgoingQueues;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
