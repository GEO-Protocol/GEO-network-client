#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSCONFIGURATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSCONFIGURATIONREQUESTMESSAGE_H


#include "../../base/transaction/TransactionMessage.cpp"


/**
 * This message is used by the intermediate nodes and the receiver node
 * to request final payment transaction configuration.
 *
 * For the details, please see: ParticipantsConfigurationMessage docs.
 */
class ParticipantsConfigurationRequestMessage:
    public TransactionMessage {

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const
        noexcept;
};


#endif //GEO_NETWORK_CLIENT_PARTICIPANTSCONFIGURATIONREQUESTMESSAGE_H
