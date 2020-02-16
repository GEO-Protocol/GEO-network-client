#include "RemoveOutdatedCryptoDataCommand.h"

RemoveOutdatedCryptoDataCommand::RemoveOutdatedCryptoDataCommand(
    const CommandUUID &uuid,
    const string &command):

    BaseUserCommand(
        uuid,
        identifier())
{
    mVacuum = false;
    auto vacuumParse = [&](auto &ctx) {
        if (_attr(ctx) == 1) {
            mVacuum = true;
        }
    };

    try {
        parse(
            command.begin(),
            command.end(),
            *(int_[vacuumParse]> eol > eoi));
    } catch(...) {
        throw ValueError("RemoveOutdatedCryptoDataCommand: cannot parse command.");
    }
}

const string &RemoveOutdatedCryptoDataCommand::identifier()
{
    static const string identifier = "DELETE:outdated-crypto";
    return identifier;
}

bool RemoveOutdatedCryptoDataCommand::vacuum() const
{
    return mVacuum;
}