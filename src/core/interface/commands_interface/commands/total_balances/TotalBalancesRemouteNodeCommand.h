#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class TotalBalancesRemouteNodeCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesRemouteNodeCommand> Shared;

public:
    TotalBalancesRemouteNodeCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    CommandResult::SharedConst responseOk(
        string &totalBalancesStr) const;

protected:
    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H
