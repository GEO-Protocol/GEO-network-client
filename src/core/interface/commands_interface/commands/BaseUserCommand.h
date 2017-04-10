#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../../../common/Types.h"
#include "../../../common/time/TimeUtils.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../CommandUUID.h"
#include "../../results_interface/result/CommandResult.h"

#include <boost/uuid/uuid.hpp>

#include <string>
#include <memory>
#include <utility>

namespace uuids = boost::uuids;

class BaseUserCommand {
public:
    typedef shared_ptr<BaseUserCommand> Shared;

public:
    static const constexpr char kCommandsSeparator = '\n';
    static const constexpr char kTokensSeparator = '\t';

public:
    BaseUserCommand(
        const string& identifier);

    BaseUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier);

    virtual ~BaseUserCommand() = default;

    const CommandUUID &UUID() const;

    const string &identifier() const;

    const DateTime &timestampAccepted() const;

    [[deprecated]]
    CommandResult::SharedConst unexpectedErrorResult();

protected:
    virtual pair<BytesShared, size_t> serializeToBytes();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    virtual void parse(
        const string &commandBuffer) = 0;

    static const size_t kOffsetToInheritedBytes();

    CommandResult::SharedConst makeResult(
        const uint16_t code) const;

private:
    CommandUUID mCommandUUID;
    DateTime mTimestampAccepted;
    const string mCommandIdentifier;
};
#endif //GEO_NETWORK_CLIENT_COMMAND_H
