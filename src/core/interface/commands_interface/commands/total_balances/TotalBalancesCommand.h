#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H

#include "../BaseUserCommand.h"

class TotalBalancesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesCommand> Shared;

public:
    TotalBalancesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &totalBalancesStr) const;

protected:

    void parse(
        const string &command);

};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
