#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../../../common/Types.h"

#include "../CommandUUID.h"

#include "../../results_interface/result/CommandResult.h"

#include <boost/uuid/uuid.hpp>

#include <string>
#include <memory>

namespace uuids = boost::uuids;

class BaseUserCommand {
public:
    typedef shared_ptr<BaseUserCommand> Shared;

public:
    BaseUserCommand();

    BaseUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier);

    const CommandUUID &commandUUID() const;

    const string &commandIdentifier() const;

    const Timestamp &timestampAccepted() const;

    const CommandResult *unexpectedErrorResult();

protected:
    virtual void deserialize(
        const string &commandBuffer) = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

    pair<BytesShared, size_t> serializeParentToBytes();

    void deserializeParentFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritBytes();

public:
    static const constexpr char kCommandsSeparator = '\n';
    static const constexpr char kTokensSeparator = '\t';

protected:
    CommandUUID mCommandUUID;
    Timestamp mTimestampAccepted;

private:
    static const Timestamp kEpoch();

    const string mCommandIdentifier;

};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
