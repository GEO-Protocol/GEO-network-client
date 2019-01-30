#include "ObservingCommunicator.h"

ObservingCommunicator::ObservingCommunicator(
    IOService &ioService,
    Logger &logger):
    mIOService(ioService),
    mLogger(logger)
{}

BytesShared ObservingCommunicator::sendRequestToObserver(
    IPv4WithPortAddress::Shared observerAddress,
    ObservingMessage::Shared request)
{
    debug() << "sendRequestToObserver " << observerAddress->fullAddress();
    tcp::resolver resolver(mIOService);
    tcp::resolver::query query(
        tcp::v4(),
        observerAddress->host(),
        to_string(observerAddress->port()));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket socket(mIOService);
    boost::asio::connect(socket, iterator);

    auto serializedRequest = request->serializeToBytes();
    boost::asio::write(
        socket,
        boost::asio::buffer(
            serializedRequest.get(),
            request->serializedSize()));
    debug() << "request sent";

    ObservingMessage::MessageSize replySize;
    boost::asio::read(
        socket,
        boost::asio::buffer(
            &replySize,
            sizeof(ObservingMessage::MessageSize)));
    debug() << "obtained reply size " << replySize;

    byte reply[replySize];
    boost::asio::read(
        socket,
        boost::asio::buffer(
            reply,
            replySize));

    socket.close();

    BytesShared result = tryMalloc(replySize);
    memcpy(
        result.get(),
        reply,
        replySize);

    return result;
}

string ObservingCommunicator::logHeader()
{
    return "[ObservingCommunicator]";
}

LoggerStream ObservingCommunicator::error() const
{
    return mLogger.error(logHeader());
}

LoggerStream ObservingCommunicator::debug() const
{
    return mLogger.debug(logHeader());
}

LoggerStream ObservingCommunicator::info() const
{
    return mLogger.info(logHeader());
}