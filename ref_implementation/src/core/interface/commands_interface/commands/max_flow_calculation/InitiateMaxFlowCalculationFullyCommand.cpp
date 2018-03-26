/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "InitiateMaxFlowCalculationFullyCommand.h"

InitiateMaxFlowCalculationFullyCommand::InitiateMaxFlowCalculationFullyCommand(
    const CommandUUID &uuid,
    const string &command):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = NodeUUID::kHexSize + 2;
    if (command.size() < minCommandLength) {
        throw ValueError("InitiateMaxFlowCalculationFullyCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }
    size_t tokenSeparatorPos = command.find(kTokensSeparator);
    string contractorsCountStr = command.substr(
        0,
        tokenSeparatorPos);
    if (contractorsCountStr.at(0) == '-') {
        throw ValueError("InitiateMaxFlowCalculationFullyCommand::parse: "
                             "Can't parse command. 'count contractors' token can't be negative.");
    }
    try {
        mContractorsCount = std::stoul(contractorsCountStr);
    } catch (...) {
        throw ValueError("InitiateMaxFlowCalculationFullyCommand::parse: "
                             "Can't parse command. Error occurred while parsing  'count contractors' token.");
    }
    mContractors.reserve(mContractorsCount);
    size_t contractorStartPoint = tokenSeparatorPos + 1;
    for (size_t idx = 0; idx < mContractorsCount; idx++) {
        try {
            string hexUUID = command.substr(
                contractorStartPoint,
                NodeUUID::kHexSize);
            mContractors.push_back(
                boost::lexical_cast<uuids::uuid>(
                    hexUUID));
            contractorStartPoint += NodeUUID::kHexSize + 1;
        } catch (...) {
            throw ValueError("InitiateMaxFlowCalculationFullyCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
        }
    }
    if (contractorStartPoint + 1 < command.length()) {
        throw ValueError("InitiateMaxFlowCalculationFullyCommand::parse: "
                             "Can't parse command. Disparity between command count contractors and real count contractors.");
    }
}

InitiateMaxFlowCalculationFullyCommand::InitiateMaxFlowCalculationFullyCommand(
    BytesShared buffer) :
    BaseUserCommand(identifier())
{
    deserializeFromBytes(buffer);
}

const string &InitiateMaxFlowCalculationFullyCommand::identifier()
{
    static const string identifier = "GET:contractors/transactions/max/fully";
    return identifier;
}

const vector<NodeUUID>& InitiateMaxFlowCalculationFullyCommand::contractors() const
{
    return mContractors;
}

CommandResult::SharedConst InitiateMaxFlowCalculationFullyCommand::responseOk(
        string &maxFlowAmount) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            maxFlowAmount));
}
