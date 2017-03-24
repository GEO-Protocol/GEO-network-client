#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class ResultRoutingTablesMessage : public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTablesMessage> Shared;

public:

    ResultRoutingTablesMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        vector<pair<NodeUUID, TrustLineDirection>> rt1);

    ResultRoutingTablesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:

    void deserializeFromBytes(
            BytesShared buffer);

private:

    vector<pair<NodeUUID, TrustLineDirection>> mRT1;

};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H
