#ifndef GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
#define GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H

#include "BaseFIFOInterface.h"
#include "../commands/Command.h"
#include "../commands/CommandsAPI.h"
#include "../common/exceptions/IOError.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>

#include <sys/types.h>

#ifdef DEBUG
#include <iostream>
#endif


using namespace std;

namespace as = boost::asio;
namespace fs = boost::filesystem;
namespace uuids = boost::uuids;


// Note: shares almost the same logic as "MessagesParser"
// (see IncomingMessagesHandler.h for details).
class CommandsParser {
public:
    pair<bool, shared_ptr<Command>> processReceivedCommand(
        const char* commandPart, const size_t receivedBytesCount);

protected:
    inline pair<bool, shared_ptr<Command>> tryDeserializeCommand();
    inline pair<bool, shared_ptr<Command>> tryParseCommand(
        const uuids::uuid &commandUUID, const string &commandIdentifier);
    inline pair<bool, shared_ptr<Command>> invalidMessageReturnValue();

    void cutCommandFromTheBuffer();

protected:
    static const size_t kUUIDHexRepresentationSize = 36;

    // uuid of the command, the space,
    // and, at least, one symbol of command identifier.
    static const size_t kMinCommandSize = kUUIDHexRepresentationSize + 2;
    static const char kCommandsSeparator = '\n';
    static const char kTokensSeparator = ' ';

protected:
    // Commands may arrive via pipe partially.
    // This buffer is needed to collect all the parts
    // and deserialize whole the message.
    //
    // This buffer is separated from the boost::asio buffer,
    // that is used for reading info from the pipe.
    string mBuffer;
};


class CommandsInterface: public BaseFIFOInterface {
public:
    explicit CommandsInterface(as::io_service &ioService, CommandsAPI *API);
    ~CommandsInterface();

    void beginAcceptCommands();

private:
    void createFIFO();
    void asyncReceiveNextCommand();
    void handleReceivedInfo(const boost::system::error_code &error,
                            const size_t bytesTransferred);

private:
    CommandsParser *mCommandsParser;
    CommandsAPI *mAPI;

    as::io_service &mIOService;
    as::posix::stream_descriptor *mFIFOStreamDescriptor;
    vector<char> mCommandBuffer;
};


#endif //GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
