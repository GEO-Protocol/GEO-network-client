#ifndef GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
#define GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H

#include "../BaseFIFOInterface.h"
#include "../../commands/Command.h"
#include "../../commands/OpenTrustLineCommand.h"
#include "../../commands/CloseTrustLineCommand.h"
#include "../../commands/UpdateOutgoingTrustAmountCommand.h"
#include "../../commands/UseCreditCommand.h"
#include "../../commands/MaximalTransactionAmountCommand.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>
#include <chrono>

#include <sys/types.h>

#ifdef DEBUG
#include <iostream>
#endif


using namespace std;

namespace as = boost::asio;
namespace fs = boost::filesystem;
namespace uuids = boost::uuids;

/*!
 * User commands are transmitted via text protocol.
 * CommandsParser is used for parsing received user input
 * and deserialising them into commands instances.
 *
 *
 * Note: shares almost the same logic as "MessagesParser"
 * (see IncomingMessagesHandler.h for details).
 */
class CommandsParser {
    friend class CommandsParserTests;

public:
    pair<bool, shared_ptr<Command>> processReceivedCommandPart(const char *commandPart,
                                                               const size_t receivedBytesCount);

protected:
    inline pair<bool, shared_ptr<Command>> tryDeserializeCommand();

    inline pair<bool, shared_ptr<Command>> tryParseCommand(const uuids::uuid &commandUUID,
                                                           const string &commandIdentifier,
                                                           const string &buffer);

    inline pair<bool, shared_ptr<Command>> commandIsInvalidOrIncomplete();

    void cutNextCommandFromTheBuffer();

protected:
    static const size_t kUUIDHexRepresentationSize = 36;
    static const size_t kMinCommandSize = kUUIDHexRepresentationSize + 2;

    static const char kCommandsSeparator = '\n';
    static const char kTokensSeparator = '\r';

    static const constexpr char* kTrustLinesOpenIdentifier = "CREATE:contractors/trust-lines";
    static const constexpr char* kTrustLinesCloseIdentifier = "REMOVE:contractors/trust-lines";
    static const constexpr char* kTrustLinesUpdateIdentifier = "SET:contractors/trust-lines";
    static const constexpr char* kTransactionsUseCreditIdentifier = "CREATE:contractors/transations";
    static const constexpr char* kTransactionsMaximalAmountIdentifier = "GET:contractors/transations/max";

protected:
    // Commands may arrive via pipe partially.
    // This buffer is needed to collect all the parts
    // and deserialize whole the message.
    //
    // This buffer is separated from the boost::asio buffer,
    // that is used for reading info from the pipe.
    string mBuffer;
};

/*!
 * User commands are transamitted via named pipe (FIFO in Linux).
 * This class is used to asyncornously receive them,
 * parse, and transfer for the further execution.
 */
class CommandsInterface : public BaseFIFOInterface {
public:
    explicit CommandsInterface(as::io_service &ioService);

    ~CommandsInterface();

    void beginAcceptCommands();

private:
    void createFIFO();

    void asyncReceiveNextCommand();

    void handleReceivedInfo(const boost::system::error_code &error,
                            const size_t bytesTransferred);

private:
    CommandsParser *mCommandsParser;

    as::io_service &mIOService;
    as::posix::stream_descriptor *mFIFOStreamDescriptor;
    vector<char> mCommandBuffer;
};

#endif //GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
