/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H

#include "../base/transaction/DestinationMessage.h"

class CloseOutgoingTrustLineMessage : public DestinationMessage {

public:
    typedef shared_ptr<CloseOutgoingTrustLineMessage> Shared;
    typedef shared_ptr<const CloseOutgoingTrustLineMessage> ConstShared;

public:
    using DestinationMessage::DestinationMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
