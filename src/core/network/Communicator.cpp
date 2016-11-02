#include "Communicator.h"

Communicator::Communicator(){
    mOutgoingMessagesHandler = new OutgoingMessagesHandler();
}

OutgoingMessagesHandler *Communicator::outgoingMessagesHandler() const {
    return mOutgoingMessagesHandler;
}
