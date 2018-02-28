#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSCOMMAND_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class GetFirstLevelContractorsCommand :
    public BaseUserCommand {

public:
    typedef shared_ptr<GetFirstLevelContractorsCommand> Shared;

public:
    GetFirstLevelContractorsCommand(
        const CommandUUID &uuid,
        const string &commandBuffer)
        noexcept;

    static const string &identifier();

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst resultOk(
        string &neighbors) const;

private:
    SerializedEquivalent mEquivalent;

};
#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSCOMMAND_H
