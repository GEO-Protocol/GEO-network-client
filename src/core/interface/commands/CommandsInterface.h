#ifndef GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
#define GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H

#include "commands/BaseUserCommand.h"
#include "commands/OpenTrustLineCommand.h"
#include "commands/CloseTrustLineCommand.h"
#include "commands/UpdateTrustLineCommand.h"
#include "commands/UseCreditCommand.h"
#include "commands/MaximalTransactionAmountCommand.h"
#include "commands/ContractorsListCommand.h"
#include "commands/TotalBalanceCommand.h"
#include "../BaseFIFOInterface.h"
#include "../../transactions/TransactionsManager.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/MemoryError.h"
#include "../../common/exceptions/RuntimeError.h"
#include "../../logger/Logger.h"

#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>
#include <chrono>
#include <exception>


using namespace std;
namespace uuids = boost::uuids;

/*!
 * User commands are transmitted via text protocol.
 * CommandsParser is used for parsing received user input
 * and deserialising them into commands instances.
 *
 *
 * Note: shares almost the same logic as "MessagesParser"
 * (see IncomingMessagesHandler.h for details).
 *
 * todo: HSC: review this class
 */
class CommandsParser {
    friend class CommandsParserTests;

public:
    static const size_t kUUIDHexRepresentationSize = 36;
    static const size_t kMinCommandSize = kUUIDHexRepresentationSize + 2;

public:
    pair<bool, shared_ptr<BaseUserCommand>> processReceivedCommandPart(
        const char *commandPart,
        const size_t receivedBytesCount);

protected:
    inline pair<bool, shared_ptr<BaseUserCommand>> tryDeserializeCommand();
    inline pair<bool, shared_ptr<BaseUserCommand>> tryParseCommand(
        const CommandUUID &uuid,
        const string &identifier,
        const string &buffer);

    inline pair<bool, shared_ptr<BaseUserCommand>> commandIsInvalidOrIncomplete();
    void cutBufferUpToNextCommand();

protected:
    static const char kCommandsSeparator = '\n';
    static const char kTokensSeparator = '\t';

    static const constexpr char* kTrustLinesOpenIdentifier = "CREATE:contractors/trust-lines";
    static const constexpr char* kTrustLinesCloseIdentifier = "REMOVE:contractors/trust-lines";
    static const constexpr char* kTrustLinesUpdateIdentifier = "SET:contractors/trust-lines";
    static const constexpr char* kTransactionsUseCreditIdentifier = "CREATE:contractors/transations";
    static const constexpr char* kTransactionsMaximalAmountIdentifier = "GET:contractors/transations/max";
    static const constexpr char* kContractorsGetAllContractorsIdentifier = "GET:contractors";
    static const constexpr char* kBalanceGetTotalBalanceIdentifier = "GET:stats/balances/total/";

protected:
    string mBuffer;
};

/*!
 * User commands are transmitted via named pipe (FIFO on Linux).
 * This class is used to asynchronously receive them, parse,
 * and transfer for the further execution.
 */
class CommandsInterface:
    public BaseFIFOInterface {

public:
    static const constexpr char *kFIFOName = "commands.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

public:
    explicit CommandsInterface(
        as::io_service &ioService,
        TransactionsManager *transactionsManager,
        Logger *logger);

    ~CommandsInterface();

    void beginAcceptCommands();

protected:
    static const constexpr size_t kCommandBufferSize = 128;

protected:
    // External
    as::io_service &mIOService;
    TransactionsManager *mTransactionsManager;
    Logger *mLog;

    // Internal
    as::posix::stream_descriptor *mFIFOStreamDescriptor;
    as::deadline_timer *mReadTimeoutTimer;
    CommandsParser *mCommandsParser;

    vector<char> mCommandBuffer;

protected:
    virtual const char* name() const;
    void asyncReceiveNextCommand();
    void handleReceivedInfo(
        const boost::system::error_code &error,
        const size_t bytesTransferred);
    void handleTimeout(
        const boost::system::error_code &error);
};

#endif //GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
