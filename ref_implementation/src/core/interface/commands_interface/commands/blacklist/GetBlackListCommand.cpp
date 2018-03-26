/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "GetBlackListCommand.h"

GetBlackListCommand::GetBlackListCommand(
    const CommandUUID &uuid,
    const string& commandBuffer)
    noexcept:
    BaseUserCommand(
        uuid,
        identifier())
{}

const string &GetBlackListCommand::identifier()
{
    static const string identifier = "GET:blacklist";
    return identifier;
}

CommandResult::SharedConst GetBlackListCommand::resultOk(
    string& bannedUsers) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        bannedUsers);
}

