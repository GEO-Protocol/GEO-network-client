/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../../../common/Types.h"
#include "../../../common/time/TimeUtils.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../results_interface/result/CommandResult.h"

#include <boost/uuid/uuid.hpp>


namespace uuids = boost::uuids;

class BaseUserCommand {
public:
    typedef shared_ptr<BaseUserCommand> Shared;

public:
    BaseUserCommand(
        const string& identifier);

    BaseUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier);

    virtual ~BaseUserCommand() = default;

    const CommandUUID &UUID() const;

    const string &identifier() const;

    // TODO: remove noexcept
    // TODO: split methods into classes
    CommandResult::SharedConst responseOK() const
        noexcept;
    CommandResult::SharedConst responseCreated() const
        noexcept;
    CommandResult::SharedConst responsePostponedByReservations() const
        noexcept;
    CommandResult::SharedConst responseProtocolError() const
        noexcept;
    CommandResult::SharedConst responseTrustlineIsAbsent() const
        noexcept;
    CommandResult::SharedConst responseCurrentIncomingDebtIsGreaterThanNewAmount() const
        noexcept;
    CommandResult::SharedConst responseTrustlineIsAlreadyPresent() const
        noexcept;

    CommandResult::SharedConst responseTrustlineRejected() const;

    CommandResult::SharedConst responseInsufficientFunds() const
        noexcept;
    CommandResult::SharedConst responseConflictWithOtherOperation() const
        noexcept;
    CommandResult::SharedConst responseRemoteNodeIsInaccessible() const
        noexcept;
    CommandResult::SharedConst responseNoRoutes() const
        noexcept;
    CommandResult::SharedConst responseUnexpectedError() const
        noexcept;
    // this response used during disabling start payment and trust line transactions
    CommandResult::SharedConst responseForbiddenRunTransaction() const
        noexcept;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

    CommandResult::SharedConst makeResult(
        const uint16_t code) const;

private:
    const CommandUUID mCommandUUID;
    DateTime mTimestampAccepted;
    const string mCommandIdentifier;
};
#endif //GEO_NETWORK_CLIENT_COMMAND_H
