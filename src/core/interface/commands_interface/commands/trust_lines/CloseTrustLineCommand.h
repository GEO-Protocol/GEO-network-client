#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../results_interface/result/CommandResult.h"

#include "../../../../common/exceptions/ValueError.h"


class CloseTrustLineCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<CloseTrustLineCommand> Shared;

public:
    CloseTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier()
        noexcept;

    const NodeUUID &contractorUUID() const
        noexcept;

    // ToDo: must be removed, or moved to the protected
    [[deprecated]]
    static size_t kRequestedBufferSize();

    [[deprecated("Remove it when parent class would be updated")]]
    virtual void parse(
        const string &_){}

private:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
