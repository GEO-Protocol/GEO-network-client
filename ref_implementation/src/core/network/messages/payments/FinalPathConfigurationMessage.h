/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_FINALPATHCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALPATHCONFIGURATIONMESSAGE_H

#include "base/RequestMessage.h"

class FinalPathConfigurationMessage : public RequestMessage {

public:
    typedef shared_ptr<FinalPathConfigurationMessage> Shared;
    typedef shared_ptr<const FinalPathConfigurationMessage> ConstShared;

public:
    using RequestMessage::RequestMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_FINALPATHCONFIGURATIONMESSAGE_H
