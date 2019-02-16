#ifndef GEO_NETWORK_CLIENT_OBSERVINGCLAIMAPPENDRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGCLAIMAPPENDRESPONSEMESSAGE_H

#include "base/ObservingResponseMessage.h"

class ObservingClaimAppendResponseMessage : public ObservingResponseMessage {

public:
    typedef shared_ptr<ObservingClaimAppendResponseMessage> Shared;

public:
    ObservingClaimAppendResponseMessage(
        BytesShared buffer);

    ObservingTransaction::ObservingResponseType observingResponse() const;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGCLAIMAPPENDRESPONSEMESSAGE_H
