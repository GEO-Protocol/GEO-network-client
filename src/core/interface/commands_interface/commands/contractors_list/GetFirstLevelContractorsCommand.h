#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSCOMMAND_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

// TODO: should be removed after testing getting paths
class GetFirstLevelContractorsCommand : public BaseUserCommand {

public:
    typedef shared_ptr<GetFirstLevelContractorsCommand> Shared;

public:
    GetFirstLevelContractorsCommand(
            const CommandUUID &uuid,
            const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &neighbors) const;

protected:
    void parse(
            const string &command);
};
#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSCOMMAND_H
