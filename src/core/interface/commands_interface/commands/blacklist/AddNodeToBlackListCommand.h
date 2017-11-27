#ifndef GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTCOMMAND_H
#define GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTCOMMAND_H


#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"


class AddNodeToBlackListCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<AddNodeToBlackListCommand> Shared;

public:
    AddNodeToBlackListCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    AddNodeToBlackListCommand(
        const CommandUUID &commandUUID,
        const NodeUUID &nodeUUID);


    static const string &identifier()
        noexcept;

    const NodeUUID& contractorUUID()
        noexcept;

//    CommandResult::SharedConst resultOk(
//        string &neighbor) const;

protected:
    [[deprecated]]
    virtual void parse(
        const string &){}

protected:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTCOMMAND_H
