#include "BaseTrustLineMessage.h"


MessageResult::SharedConst BaseTrustLineMessage::customCodeResult(
    const uint16_t code) const
{
    return make_shared<const MessageResult>(
        senderUUID,
        mTransactionUUID,
        code);
}