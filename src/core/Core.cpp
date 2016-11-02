#include "Core.h"
#include "network/messages/TestMessage.h"

Core::Core() {
    mCommunicator = new Communicator();
}

int Core::run() {
    auto nodeUUID = std::shared_ptr<NodeUUID>(new NodeUUID());
    auto message  = std::shared_ptr<Message>(new TestMessage());

    try {
        mCommunicator->outgoingMessagesHandler()->sendMessageToTheNode(nodeUUID, message);
        return 0;

    } catch (Exception &e) {
        std::cout << "Exception: " << e.message() << std::endl;
        return -1;
    }
}

