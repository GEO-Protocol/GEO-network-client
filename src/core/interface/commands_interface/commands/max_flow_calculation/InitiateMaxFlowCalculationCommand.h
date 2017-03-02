//
// Created by mc on 14.02.17.
//

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
            const string &commandBuffer);

    InitiateMaxFlowCalculationCommand(
            BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    pair<BytesShared, size_t> serializeToBytes();

    CommandResult::SharedConst resultOk(string &maxFlowAmount) const;

protected:
    void deserializeFromBytes(
            BytesShared buffer);

    void parse(
            const string &command);

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONCOMMAND_H
