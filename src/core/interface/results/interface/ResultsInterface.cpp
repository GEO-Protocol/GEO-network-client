#include "ResultsInterface.h"

ResultsInterface::ResultsInterface(Logger *logger) :
    mLog(logger){

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
    mFIFODescriptor = open(FIFOFilePath().c_str(), O_RDWR | O_NONBLOCK);
    if (mFIFODescriptor == -1) {
        throw IOError(
            "ResultsInterface::ResultsInterface: "
                "Can't open FIFO file.");
    }
}

ResultsInterface::~ResultsInterface() {
    close(mFIFODescriptor);
}

void ResultsInterface::writeResult(
    const char *bytes,
    const size_t bytesCount) {

#ifdef DEBUG_RESULTS_INTERFACE
    mLog->logInfo("Results interface: message received", bytes);
#endif

    if (write(mFIFODescriptor, bytes, bytesCount) == bytesCount ||
        write(mFIFODescriptor, bytes, bytesCount) == bytesCount) {
        fsync(mFIFODescriptor);

    } else {
        throw IOError(
            "ResultsInterface::writeResult: "
                "can't write resultto the disk.");
    }
}

const char *ResultsInterface::name() const {
    return kFIFOName;
}

