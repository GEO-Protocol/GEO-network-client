/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CommandResult.h"

CommandResult::CommandResult(
    const string &commandIdentifier,
    const CommandUUID &commandUUID,
    const uint16_t resultCode) :

    mCommandUUID(commandUUID),
    mTimestampCompleted(utc_now()),
    mCommandIdentifier(commandIdentifier)
{
    mResultCode = resultCode;
}

CommandResult::CommandResult(
    const string &commandIdentifier,
    const CommandUUID &commandUUID,
    const uint16_t resultCode,
    string &resultInformation) :

    mCommandUUID(commandUUID),
    mTimestampCompleted(utc_now()),
    mResultInformation(resultInformation),
    mCommandIdentifier(commandIdentifier)
{
    mResultCode = resultCode;
}

const CommandUUID &CommandResult::commandUUID() const
{
    return mCommandUUID;
}

const uint16_t CommandResult::resultCode() const
{
    return mResultCode;
}

const DateTime &CommandResult::timestampCompleted() const
{
    return mTimestampCompleted;
}

const string CommandResult::serialize() const
{
    if (!mResultInformation.empty()) {
        return mCommandUUID.stringUUID() + kTokensSeparator +
               to_string(mResultCode) + kTokensSeparator +
               mResultInformation + kCommandsSeparator;
    }
    return mCommandUUID.stringUUID() + kTokensSeparator +
           to_string(mResultCode) + kCommandsSeparator;
}

const string CommandResult::serializeShort() const
{
    return mCommandUUID.stringUUID() + kTokensSeparator +
           to_string(mResultCode) + kCommandsSeparator;
}

const string CommandResult::identifier() const
{
    return mCommandIdentifier;
}
