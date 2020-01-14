#include "RemoveOutdatedCryptoDataCommand.h"

RemoveOutdatedCryptoDataCommand::RemoveOutdatedCryptoDataCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{}

const string &RemoveOutdatedCryptoDataCommand::identifier()
{
    static const string identifier = "DELETE:outdated-crypto";
    return identifier;
}