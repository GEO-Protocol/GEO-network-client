/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 2;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError("TotalBalancesCommand: "
                             "Can't parse command. Received command is to short.");
    }
    size_t tokenSeparatorPos = commandBuffer.find(kTokensSeparator);
    string gatewaysCountStr = commandBuffer.substr(
        0,
        tokenSeparatorPos);
    if (gatewaysCountStr.at(0) == '-') {
        throw ValueError("TotalBalancesCommand: "
                             "Can't parse command. 'count gateways' token can't be negative.");
    }
    try {
        mGatewaysCount = std::stoul(gatewaysCountStr);
    } catch (...) {
        throw ValueError("TotalBalancesCommand: "
                             "Can't parse command. Error occurred while parsing  'count gateways' token.");
    }
    if (mGatewaysCount == 0) {
        return;
    }
    mGateways.reserve(mGatewaysCount);
    size_t gatewayStartPoint = tokenSeparatorPos + 1;
    for (size_t idx = 0; idx < mGatewaysCount; idx++) {
        try {
            string hexUUID = commandBuffer.substr(
                gatewayStartPoint,
                NodeUUID::kHexSize);
            mGateways.push_back(
                boost::lexical_cast<uuids::uuid>(
                    hexUUID));
            gatewayStartPoint += NodeUUID::kHexSize + 1;
        } catch (...) {
            throw ValueError("TotalBalancesCommand: "
                                 "Can't parse command. Error occurred while parsing 'Gateway UUID' token.");
        }
    }
    if (gatewayStartPoint + 1 < commandBuffer.length()) {
        throw ValueError("TotalBalancesCommand: "
                             "Can't parse command. Disparity between command count gateways and real count gateways.");
    }
}

const string &TotalBalancesCommand::identifier()
{
    static const string identifier = "GET:stats/balance/total";
    return identifier;
}

const vector<NodeUUID>& TotalBalancesCommand::gateways() const
{
    return mGateways;
}

CommandResult::SharedConst TotalBalancesCommand::resultOk(
    string &totalBalancesStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            totalBalancesStr));
}
