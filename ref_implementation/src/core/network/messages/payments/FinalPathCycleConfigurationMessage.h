/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H

#include "base/RequestCycleMessage.h"

class FinalPathCycleConfigurationMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<FinalPathCycleConfigurationMessage> Shared;
    typedef shared_ptr<const FinalPathCycleConfigurationMessage> ConstShared;

public:
    using RequestCycleMessage::RequestCycleMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
