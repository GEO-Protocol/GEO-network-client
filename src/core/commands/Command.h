#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;

namespace uuids = boost::uuid;
namespace multiprecision = boost::multiprecision;

typedef multiprecision::checked_uint256_t trust_amount;
typedef multiprecision::int256_t balance_value;

class Command {
public:
    Command(const string &identifier);

    Command(const string &identifier, const string &timestampExcepted);

    Command(const uuids::uuid &commandUUID, const string &identifier, const string &timestampExcepted);

    const uuids::uuid &commandsUUID() const;

    const string &identifier() const;

    const string &timeStampExcepted() const;

protected:
    virtual void deserialize() = 0;

public:
    static const size_t kUUIDHexSize = 36;
    static const char kCommandsSeparator = '\n';
    static const char kTokensSeparator = ' ';

private:
    uuids::uuid mCommandUUID;

    string mIdentifier;

    string mTimestampExcepted;
};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
