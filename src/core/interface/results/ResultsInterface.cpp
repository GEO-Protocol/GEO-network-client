#include "ResultsInterface.h"




//ResultsInterface::ResultsInterface() {
//    obtainFileDescriptor();
//}
//
//ResultsInterface::~ResultsInterface() {
//    if (mFileDescriptor != NULL) {
//        fclose(mFileDescriptor);
//    }
//}
//
//void ResultsInterface::writeResult(const string &result) {
//    checkFileDescriptor();
//    const byte *buffer = reinterpret_cast<const byte*>(result.c_str());
//    size_t length = result.size();
//    fseek(mFileDescriptor, 0, SEEK_END);
//    if (fwrite(buffer, 1, length, mFileDescriptor) != length){
//        if (fwrite(buffer, 1, length, mFileDescriptor) != length){
//            throw IOError(string("Can't write result buffer in file.").c_str());
//        }
//    }
//}
//
//void ResultsInterface::obtainFileDescriptor() {
//    if (isFileExists()){
//        mFileDescriptor = fopen(mFileName.c_str(), kModeUpdate.c_str());
//        checkFileDescriptor();
//    } else {
//        mFileDescriptor = fopen(mFileName.c_str(), kModeCreate.c_str());
//        checkFileDescriptor();
//    }
//}
//
//void ResultsInterface::checkFileDescriptor() {
//    if (mFileDescriptor == NULL) {
//        throw IOError(string("Unable to obtain file descriptor.").c_str());
//    }
//}
//
//const bool ResultsInterface::isFileExists() {
//    return fs::exists(fs::path(mFileName.c_str()));
//}
//


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
    mFIFODescriptor = open(FIFOFilePath().c_str(), O_RDONLY | O_NONBLOCK);
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
        as::transfer_all());
}

const char *ResultsInterface::name() const {
    return kFIFOName;
}
