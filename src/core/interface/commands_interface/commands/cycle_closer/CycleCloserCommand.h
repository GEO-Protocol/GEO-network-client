#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERCOMMAND_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"
#include "../../../../paths/lib/Path.h"

// TODO: should be removed after testing closing cycles
class CycleCloserCommand : public BaseUserCommand {

public:
    typedef shared_ptr<CycleCloserCommand> Shared;

public:
    CycleCloserCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk() const;

    Path::ConstShared path();

protected:
    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_CYCLECLOSERCOMMAND_H
