#ifndef GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONSRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONSRESPONSEMESSAGE_H

#include "ObservingMessage.hpp"
#include "../ObservingTransaction.h"
#include <vector>

class ObservingTransactionsResponseMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingTransactionsResponseMessage> Shared;

public:
    ObservingTransactionsResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    BlockNumber actualBlockNumber() const;

    vector<ObservingTransaction::ObservingResponseType> transactionsResponses() const;

private:
    BlockNumber mActualBlockNumber;
    vector<ObservingTransaction::ObservingResponseType> mTransactionsAndResponses;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONSRESPONSEMESSAGE_H
