/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H

#include "base/ResponseMessage.h"


class ReceiverInitPaymentResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<ReceiverInitPaymentResponseMessage> Shared;
    typedef shared_ptr<const ReceiverInitPaymentResponseMessage> ConstShared;

public:
    using  ResponseMessage::ResponseMessage;

protected:
    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H
