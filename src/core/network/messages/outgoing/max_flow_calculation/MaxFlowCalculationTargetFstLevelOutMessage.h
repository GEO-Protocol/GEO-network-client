//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELOUTMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELOUTMESSAGE_H

#include "../../MaxFlowCalculationMessage.hpp"

class MaxFlowCalculationTargetFstLevelOutMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelOutMessage> Shared;

public:
    MaxFlowCalculationTargetFstLevelOutMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELOUTMESSAGE_H
