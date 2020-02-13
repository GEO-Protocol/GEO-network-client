#ifndef GEO_NETWORK_CLIENT_REMOVEOUTDATEDCRYPTODATACOMMAND_H
#define GEO_NETWORK_CLIENT_REMOVEOUTDATEDCRYPTODATACOMMAND_H

#include "../BaseUserCommand.h"

class RemoveOutdatedCryptoDataCommand : public BaseUserCommand {

public:
    typedef shared_ptr<RemoveOutdatedCryptoDataCommand> Shared;

public:
    RemoveOutdatedCryptoDataCommand(
        const CommandUUID &uuid,
        const string &command);

    static const string &identifier();

    bool vacuum() const;

private:
    bool mVacuum;
};


#endif //GEO_NETWORK_CLIENT_REMOVEOUTDATEDCRYPTODATACOMMAND_H
