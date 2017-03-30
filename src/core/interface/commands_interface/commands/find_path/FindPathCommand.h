#ifndef GEO_NETWORK_CLIENT_FINDPATHCOMMAND_H
#define GEO_NETWORK_CLIENT_FINDPATHCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class FindPathCommand : public BaseUserCommand {

public:
    typedef shared_ptr<FindPathCommand> Shared;

public:
    FindPathCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    CommandResult::SharedConst resultOk(string &path) const;

    CommandResult::SharedConst resultNoPath() const;

    CommandResult::SharedConst resultNoResponse() const;

protected:
    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_FINDPATHCOMMAND_H
