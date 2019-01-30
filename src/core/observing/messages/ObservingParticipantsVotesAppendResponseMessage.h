#ifndef GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESAPPENDRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESAPPENDRESPONSEMESSAGE_H

#include "ObservingMessage.hpp"
#include "../ObservingTransaction.h"

class ObservingParticipantsVotesAppendResponseMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingParticipantsVotesAppendResponseMessage> Shared;

public:
    ObservingParticipantsVotesAppendResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    ObservingTransaction::ObservingResponseType observingResponse() const;

private:
    ObservingTransaction::ObservingResponseType mObservingResponse;

};


#endif //GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESAPPENDRESPONSEMESSAGE_H
