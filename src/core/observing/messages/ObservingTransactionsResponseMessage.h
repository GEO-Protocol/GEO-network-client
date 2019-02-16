#ifndef GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONSRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONSRESPONSEMESSAGE_H

#include "base/ObservingResponseMessage.h"
#include <vector>

class ObservingTransactionsResponseMessage : public ObservingResponseMessage {

public:
    typedef shared_ptr<ObservingTransactionsResponseMessage> Shared;

public:
    ObservingTransactionsResponseMessage(
        BytesShared buffer);

    BlockNumber actualBlockNumber() const;

    vector<ObservingTransaction::ObservingResponseType> transactionsResponses() const;

private:
    ObservingTransaction::SerializedObservingResponseType mObservingResponse;
    BlockNumber mActualBlockNumber;
    vector<ObservingTransaction::ObservingResponseType> mTransactionsAndResponses;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONSRESPONSEMESSAGE_H
