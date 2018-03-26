/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONFULLYCOMMAND_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONFULLYCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class InitiateMaxFlowCalculationFullyCommand : public BaseUserCommand {
public:
    typedef shared_ptr<InitiateMaxFlowCalculationFullyCommand> Shared;

public:
    InitiateMaxFlowCalculationFullyCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    InitiateMaxFlowCalculationFullyCommand(
        BytesShared buffer);

    static const string &identifier();

    const vector<NodeUUID> &contractors() const;

    CommandResult::SharedConst responseOk(
        string &maxFlowAmount) const;

private:
    size_t mContractorsCount;
    vector<NodeUUID> mContractors;
};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONFULLYCOMMAND_H
