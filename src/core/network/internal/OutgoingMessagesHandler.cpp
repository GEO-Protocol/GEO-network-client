#include "OutgoingMessagesHandler.h"

OutgoingMessagesHandler::OutgoingMessagesHandler() {
//    mOutgoingQueues = new std::map<NodeUUID, OutgoingMessagesQueue*>();

    // todo: restore all queues on startup
}

//void OutgoingMessagesHandler::sendMessageToTheNode(
//    const std::shared_ptr<NodeUUID> nodeUUID,
//    const std::shared_ptr<Message> message) {
//
////    if (! containsQueueFor(nodeUUID))
////        initQueueFor(nodeUUID);
////
////    auto queue = queueFor(nodeUUID);
////    queue->enqueue(message);
//}

//bool OutgoingMessagesHandler::containsQueueFor(const std::shared_ptr<NodeUUID> nodeUUID) const {
//    return mOutgoingQueues->count(*nodeUUID) > 1;
//}

//void OutgoingMessagesHandler::initQueueFor(const std::shared_ptr<NodeUUID> nodeUUID) {
//    OutgoingMessagesQueue *queue = new OutgoingMessagesQueue(nodeUUID.get());
//    mOutgoingQueues->insert(std::pair<NodeUUID, OutgoingMessagesQueue*>(*nodeUUID, queue));
//}
//
//OutgoingMessagesQueue *OutgoingMessagesHandler::queueFor(const std::shared_ptr<NodeUUID> nodeUUID) const {
//    return mOutgoingQueues->at(*nodeUUID);
//}
