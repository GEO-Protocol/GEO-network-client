#include "ResultsInterface.h"

ResultsInterface::ResultsInterface(
    as::io_service &ioService):

    mIOService(ioService){

    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }

    // Try to open FIFO file in non-blocking manner.
    // In case if this file will be opened in standard blocking manner -
    // it will freeze whole the process,
    // until writer would connect to the FIFO from the other side.
    //
    // For the server realisation this makes the process unusable,
    // because it can't be demonized, until the commands writer will
    // open the commands file for writing.
    mFIFODescriptor = open(FIFOFilePath().c_str(), O_WRONLY | O_NONBLOCK);
    if (mFIFODescriptor == -1) {
        throw IOError(
            "ResultsInterface::ResultsInterface: "
                "Can't open FIFO file.");
    }

    try {
        mFIFOStreamDescriptor = new as::posix::stream_descriptor(mIOService, mFIFODescriptor);
    } catch (exception &) {
        throw MemoryError(
            "ResultsInterface::ResultsInterface: "
                "Can't allocate enough space for mFIFOStreamDescriptor. "
                "Can't open FIFO file.");
    }
}

ResultsInterface::~ResultsInterface() {
    close(mFIFODescriptor);
    delete mFIFOStreamDescriptor;
}

void ResultsInterface::writeResult(
    const char *bytes,
    const size_t bytesCount) {

    as::async_write(
        *mFIFOStreamDescriptor,
        as::buffer(vector<char>(bytes, bytes+bytesCount)),
        boost::bind(
                &ResultsInterface::handleTransferredInfo, this,
                as::placeholders::error,
                as::placeholders::bytes_transferred));
}

void ResultsInterface::handleTransferredInfo(
        const boost::system::error_code &error,
        const size_t bytesTransferred) {
    if (!error) {
        cout << "Bytes transferred " + to_string(bytesTransferred) << endl;
    } else {
        cout << error.message() << endl;
    }
}

const char *ResultsInterface::name() const {
    return kFIFOName;
}

