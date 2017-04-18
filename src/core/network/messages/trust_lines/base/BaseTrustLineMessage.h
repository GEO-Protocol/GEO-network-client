#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../result/MessageResult.h"


// ToDo: [hsc review] it seems, that this message is redundant.
// it only contains one custom result, that is possibly also redundant.
class BaseTrustLineMessage:
    public TransactionMessage {

public:
    using TransactionMessage::TransactionMessage;

    MessageResult::SharedConst customCodeResult(
        const uint16_t code) const;
};
#endif //GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
