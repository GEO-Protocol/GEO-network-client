//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H

#include "../../MaxFlowCalculationMessage.hpp"

class SendMaxFlowCalculationSourceFstLevelMessage : public  MaxFlowCalculationMessage{

public:
    typedef shared_ptr<SendMaxFlowCalculationSourceFstLevelMessage> Shared;

public:
    SendMaxFlowCalculationSourceFstLevelMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID);

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
