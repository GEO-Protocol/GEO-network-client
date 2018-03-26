/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "AddNodeToBlackListCommand.h"


AddNodeToBlackListCommand::AddNodeToBlackListCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "AddNodeToBlackListCommand: can't parse command. "
                "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "AddNodeToBlackListCommand: can't parse command. "
                "Error occurred while parsing 'Contractor UUID' token.");
    }
}

const string &AddNodeToBlackListCommand::identifier()
noexcept
{
    static const string identifier = "POST:blacklist";
    return identifier;
}

const NodeUUID &AddNodeToBlackListCommand::contractorUUID()
noexcept {
    return mContractorUUID;
}