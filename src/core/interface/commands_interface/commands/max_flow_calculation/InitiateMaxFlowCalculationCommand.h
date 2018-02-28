#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONCOMMAND_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONCOMMAND_H


#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class InitiateMaxFlowCalculationCommand : public BaseUserCommand {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationCommand> Shared;

public:
    InitiateMaxFlowCalculationCommand(
        const CommandUUID &uuid,
        const string &command);

    static const string &identifier();

    const vector<NodeUUID> &contractors() const;

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst responseOk(
        string &maxFlowAmount) const;

private:
    size_t mContractorsCount;
    vector<NodeUUID> mContractors;
    SerializedEquivalent mEquivalent;
};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONCOMMAND_H
