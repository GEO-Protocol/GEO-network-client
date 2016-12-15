#include "ResultsInterface.h"

ResultsInterface::ResultsInterface() {
    obtainFileDescriptor();
}

ResultsInterface::~ResultsInterface() {
    if (mFileDescriptor != NULL) {
        fclose(mFileDescriptor);
    }
}

void ResultsInterface::writeResult(const string &result) {
    checkFileDescriptor();
    const byte *buffer = reinterpret_cast<const byte*>(result.c_str());
    size_t length = result.size();
    fseek(mFileDescriptor, 0, SEEK_END);
    if (fwrite(buffer, 1, length, mFileDescriptor) != length){
        if (fwrite(buffer, 1, length, mFileDescriptor) != length){
            throw IOError(string("Can't write result buffer in file.").c_str());
        }
    }
}

void ResultsInterface::obtainFileDescriptor() {
    if (isFileExists()){
        mFileDescriptor = fopen(mFileName.c_str(), kModeUpdate.c_str());
        checkFileDescriptor();
    } else {
        mFileDescriptor = fopen(mFileName.c_str(), kModeCreate.c_str());
        checkFileDescriptor();
    }
}

void ResultsInterface::checkFileDescriptor() {
    if (mFileDescriptor == NULL) {
        throw IOError(string("Unable to obtain file descriptor.").c_str());
    }
}

const bool ResultsInterface::isFileExists() {
    return fs::exists(fs::path(mFileName.c_str()));
}

