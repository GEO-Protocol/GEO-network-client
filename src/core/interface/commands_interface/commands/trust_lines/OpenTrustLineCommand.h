#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../common/exceptions/ValueError.h"


class  OpenTrustLineCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<OpenTrustLineCommand> Shared;

public:
    OpenTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const TrustLineAmount &amount() const;

protected:
    [[deprecated]]
    virtual void parse(
        const string &){}

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINECOMMAND_H
