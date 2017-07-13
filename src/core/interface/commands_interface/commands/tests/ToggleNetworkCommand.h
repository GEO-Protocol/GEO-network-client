#ifndef TOGGLENETWORKCOMMAND_H
#define TOGGLENETWORKCOMMAND_H

#include "BaseTestCommand.h"

#include "../../../../common/exceptions/ValueError.h"


#ifdef TESTS

/**
 * This command is used for enabling and disabling network
 */
class ToggleNetworkCommand:
    public BaseTestCommand {

public:
    typedef shared_ptr<ToggleNetworkCommand> Shared;

public:
    ToggleNetworkCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    bool isNetworkOn() const;

protected:
    [[deprecated]]
    virtual void parse(
        const string &){}

protected:
    bool mIsNetworkOn;
};

#endif

#endif // TOGGLENETWORKCOMMAND_H
