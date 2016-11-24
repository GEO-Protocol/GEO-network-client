#include "ResultsInterface.h"

ResultsInterface::ResultsInterface(as::io_service &ioService) :
        mIOService(ioService) {
    if (!FIFOExists()) {
        createFifo();
    }
    mFIFODescriptor = open(FIFOPath().c_str(), O_WRONLY);
    if (mFIFODescriptor == -1) {
        throw IOError("Can not open FIFO file");
    }

    try {
        mFIFOStreamDescriptor = new as::posix::stream_descriptor(mIOService, mFIFODescriptor);
    } catch (const std::exception &e) {
        throw IOError("Can't assign FIFO descriptor to writer");
    }
}

ResultsInterface::~ResultsInterface() {
    close(mFIFODescriptor);
    delete mFIFOStreamDescriptor;
}

void ResultsInterface::createFifo() {
    if (!FIFOExists()) {
        fs::create_directories(dir());
        mkfifo(FIFOPath().c_str(), 0420);
    }
}

void ResultsInterface::writeNextCommand() {
    mFIFOStreamDescriptor->async_write_some(
            as::buffer(mResultBuffer),
            boost::bind(&ResultsInterface::writerHandler, this, as::placeholders::error,
                        as::placeholders::bytes_transferred)
    );
}

void ResultsInterface::writerHandler(const boost::system::error_code &error, const size_t bytesTransferred) {

}