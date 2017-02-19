//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELOUTMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELOUTMESSAGE_H

#include "../../MaxFlowCalculationMessage.hpp"

class MaxFlowCalculationSourceFstLevelOutMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelOutMessage> Shared;

public:
    MaxFlowCalculationSourceFstLevelOutMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELOUTMESSAGE_H
