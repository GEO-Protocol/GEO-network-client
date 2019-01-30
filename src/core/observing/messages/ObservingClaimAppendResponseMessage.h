#ifndef GEO_NETWORK_CLIENT_OBSERVINGCLAIMAPPENDRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGCLAIMAPPENDRESPONSEMESSAGE_H

#include "ObservingMessage.hpp"
#include "../ObservingTransaction.h"

class ObservingClaimAppendResponseMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingClaimAppendResponseMessage> Shared;

public:
    ObservingClaimAppendResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    ObservingTransaction::ObservingResponseType observingResponse() const;

private:
    ObservingTransaction::ObservingResponseType mObservingResponse;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGCLAIMAPPENDRESPONSEMESSAGE_H
