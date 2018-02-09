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
