#ifndef GEO_NETWORK_CLIENT_UPDATEROUTINGTABLESCOMMAND_H
#define GEO_NETWORK_CLIENT_UPDATEROUTINGTABLESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"


class UpdateRoutingTablesCommand :
    public BaseUserCommand {

public:
    typedef shared_ptr<UpdateRoutingTablesCommand> Shared;

public:
    UpdateRoutingTablesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer)
    noexcept;

    static const string &identifier();

protected:
    void parse(const string &command);
};
#endif //GEO_NETWORK_CLIENT_UPDATEROUTINGTABLESCOMMAND_H
