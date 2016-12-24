#include "AbstractFileDescriptorHandler.h"


namespace db {


AbstractFileDescriptorHandler::AbstractFileDescriptorHandler(
    const fs::path &path):

    mFileDescriptor(nullptr),
    mDirPath(path.parent_path()),
    mFilename(path.filename()){

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(path.has_filename());
    assert(path.has_extension());
    assert(path.has_parent_path());
#endif
}

AbstractFileDescriptorHandler::~AbstractFileDescriptorHandler() {
    close();
}

const fs::path &AbstractFileDescriptorHandler::filename() const {
    return mFilename;
}

const fs::path &AbstractFileDescriptorHandler::path() const {
    return mDirPath;
}

/*!
 * Opens file with specified "accessMode".
 *
 * Throws IOError in case when file is already opened,
 * or in case of FS error.
 */
void AbstractFileDescriptorHandler::open(
    const char *accessMode) {

    if (mFileDescriptor != nullptr) {
        throw IOError(
            "AbstractFileDescriptorHandler::open: "
                "file is already opened.");
    }

    // Create directories if doesn't exists.
    if (! fs::exists(path())) {
        fs::create_directories(path());
    }

    mFileDescriptor = fopen((path() / filename()).c_str(), accessMode);
    if (mFileDescriptor == nullptr) {
        throw IOError(
            "AbstractFileDescriptorHandler::open: "
                "File can't be opened.");
    }
}

/*!
 * Flushes the buffers and closes file descriptor.
 */
void AbstractFileDescriptorHandler::close() {
    if (mFileDescriptor != nullptr) {
        fflush(mFileDescriptor);
        fclose(mFileDescriptor);
    }
}

__off_t AbstractFileDescriptorHandler::fileSize() const {
    struct stat stbuf;
    if ((fstat(fileno(mFileDescriptor), &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
        throw IOError(
            "AbstractFileDescriptorHandler::fileSize: "
                "Can't determine the size of the file.");
    }

    return stbuf.st_size;
}

const bool AbstractFileDescriptorHandler::exists() const {
    return fs::exists((path() / filename()));
}

void AbstractFileDescriptorHandler::syncLowLevelOSBuffers() const {
    fflush(mFileDescriptor);
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "UUIDMapColumn::writeBlock: "
                "can't sync user-space buffers.");
    }
}


} // namespace db
