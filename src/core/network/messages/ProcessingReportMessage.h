#ifndef GEO_NETWORK_CLIENT_PROCESSINGREPORTMESSAGE_H
#define GEO_NETWORK_CLIENT_PROCESSINGREPORTMESSAGE_H

#include "Message.h"

#include <boost/crc.hpp>


// This message type is used to inform contractor node
// about successfully processed message.
//
// Each one message in the system should be confirmed by message of this type.
// When node receives this message - it removes one message from the messages delivery queue.
class ProcessingReportMessage: public Message {
public:
    ProcessingReportMessage(uint32_t previousMessageCRC32):
        mPreviousMessageCRC32(previousMessageCRC32){};

    virtual std::shared_ptr<SerialisedMessage> serialize() const {
        return std::shared_ptr<SerialisedMessage>(
            new SerialisedMessage(
                (uint8_t*)(&mPreviousMessageCRC32), sizeof(mPreviousMessageCRC32), typeID()));
    }

    virtual const MessageTypeID typeID() const {
        return Message::ProcessingReportMessage;
    }

private:
    uint32_t mPreviousMessageCRC32;
};

#endif //GEO_NETWORK_CLIENT_PROCESSINGREPORTMESSAGE_H
