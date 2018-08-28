/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "MaxFlowCalculationSourceSndLevelMessage.h"

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID)
{}

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    BytesShared buffer):

    MaxFlowCalculationMessage(buffer)
{}

const Message::MessageType MaxFlowCalculationSourceSndLevelMessage::typeID() const
{
    return Message::MessageType::MaxFlow_CalculationSourceSecondLevel;
}
