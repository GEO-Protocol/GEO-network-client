#ifndef GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H
#define GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H
#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class GetFirstLevelContractorsBalancesCommand :
        public BaseUserCommand {

public:
    typedef shared_ptr<GetFirstLevelContractorsBalancesCommand> Shared;

public:
    GetFirstLevelContractorsBalancesCommand(
            const CommandUUID &uuid,
            const string &commandBuffer)
    noexcept;

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &neighbors) const;

protected:
    void parse(
            const string &command);
};
#endif //GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H
