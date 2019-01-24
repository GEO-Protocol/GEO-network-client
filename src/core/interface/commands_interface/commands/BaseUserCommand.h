#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../../../common/Types.h"
#include "../../../common/time/TimeUtils.h"
#include "../../../common/memory/MemoryUtils.h"
#include "../../results_interface/result/CommandResult.h"

#include <boost/uuid/uuid.hpp>

#include <boost/spirit/home/x3.hpp>

using boost::spirit::x3::int_;
using boost::spirit::x3::char_;
using boost::spirit::x3::_attr;
using boost::spirit::x3::repeat;
using boost::spirit::x3::eol;
using boost::spirit::x3::ascii::space;
using boost::spirit::x3::ascii::digit;
using boost::spirit::x3::ascii::alpha;
using boost::spirit::x3::ascii::punct;

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
    CommandResult::SharedConst responseTrustLineIsAbsent() const
        noexcept;
    CommandResult::SharedConst responseThereAreNoKeys() const
        noexcept;
    CommandResult::SharedConst responseNoConsensus() const
        noexcept;
    CommandResult::SharedConst responseInsufficientFunds() const
        noexcept;
    CommandResult::SharedConst responseInsufficientFundsDueToKeysAbsent() const
        noexcept;
    CommandResult::SharedConst responseInsufficientFundsDueToParticipantsKeysAbsent() const
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

    CommandResult::SharedConst responseEquivalentIsAbsent() const
        noexcept;

protected:
    CommandResult::SharedConst makeResult(
        const uint16_t code) const;

private:
    const CommandUUID mCommandUUID;
    const string mCommandIdentifier;
};
#endif //GEO_NETWORK_CLIENT_COMMAND_H
