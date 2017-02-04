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
    static const constexpr char kCommandsSeparator = '\n';
    static const constexpr char kTokensSeparator = '\t';

public:
    // todo: (DM) unused, may be removed
    BaseUserCommand();

    BaseUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier);

    // todo: (DM) commandUUID() -> UUID()
    const CommandUUID &commandUUID() const;

    // todo: (DM) commandIdentifier() -> identifier()
    const string &commandIdentifier() const;

    const Timestamp &timestampAccepted() const;

    // todo: (DM) shared?
    const CommandResult *unexpectedErrorResult();

protected:
    // todo: (DM) deserialize() -> parse()
    virtual void deserialize(
        const string &commandBuffer) = 0;

    // todo: (DM) may be const
    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

    // todo: (DM) move logic from this into serializeToBytes()
    // todo: (DM) into derived classes simply call BaseUserCommand::serializeToBytes()
    pair<BytesShared, size_t> serializeParentToBytes();

    // todo: (DM) similar to the serializeParentToBytes
    void deserializeParentFromBytes(
        BytesShared buffer);

    // todo: (DM) inheritED?
    static const size_t kOffsetToInheritBytes();


protected:
    CommandUUID mCommandUUID;
    Timestamp mTimestampAccepted;

private:
    static const Timestamp kEpoch();

    const string mCommandIdentifier;
};
#endif //GEO_NETWORK_CLIENT_COMMAND_H
