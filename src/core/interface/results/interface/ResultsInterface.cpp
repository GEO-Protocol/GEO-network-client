#include "ResultsInterface.h"

ResultsInterface::ResultsInterface(Logger *logger) :
    mLog(logger){

    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }
}

void ResultsInterface::writeResult(
    const char *bytes,
    const size_t bytesCount) {

#ifdef DEBUG_RESULTS_INTERFACE
    mLog->logInfo("Results interface: message received", bytes);
#endif

    auto FIFODescriptor = open(FIFOFilePath().c_str(), O_WRONLY | O_RSYNC | O_DSYNC);
    if (FIFODescriptor == -1) {
        throw IOError(
            "ResultsInterface::ResultsInterface: "
                "Can't open FIFO file.");
    }

    if (write(FIFODescriptor, bytes, bytesCount) == bytesCount ||
        write(FIFODescriptor, bytes, bytesCount) == bytesCount) {
//        fflush(fdopen(FIFODescriptor, "a"));
        fdatasync(FIFODescriptor);
        close(FIFODescriptor);

    } else {
        close(FIFODescriptor);
        throw IOError(
            "ResultsInterface::writeResult: "
                "can't write result to the disk.");
    }
}

const char *ResultsInterface::name() const {
    return kFIFOName;
}

