#include "AbstractFileDescriptorHandler.h"


namespace db {
namespace fields {


AbstractFileDescriptorHandler::AbstractFileDescriptorHandler(const char *filename,
                                                             const char *path):
    mFileDescriptor(nullptr), mFilename(filename), mPath(path){}

AbstractFileDescriptorHandler::~AbstractFileDescriptorHandler() {
    close();
}

const string &AbstractFileDescriptorHandler::filename() const {
    return mFilename;
}

const string &AbstractFileDescriptorHandler::path() const {
    return mPath;
}

const bool AbstractFileDescriptorHandler::exists() const {
    return fs::exists(fs::path(filename()));
}

/*!
 * Opens file with specified "accessMode".
 *
 * Throws IOError in case when file is already opened,
 * or in case of FS error.
 */
void AbstractFileDescriptorHandler::open(const char *accessMode) {
    if (mFileDescriptor != nullptr) {
        throw IOError(
            "AbstractFileDescriptorHandler::open: "
                "File is already opened.");
    }

    // Create directories if doesn't exists.
    fs::path path(mPath);
    if (! fs::exists(path)) {
        fs::create_directories(path);
    }

    path.append(mFilename);
    mFileDescriptor = fopen(path.c_str(), accessMode);
    if (mFileDescriptor == nullptr) {
        throw IOError(
            "AbstractFileDescriptorHandler::open: "
                "File can't be opened.");
    }
}

/*!
 * Flushes the buffers and closes the file descriptor;
 *
 * Throws IOError in case when file is not opened.
 */
void AbstractFileDescriptorHandler::close() {
    if (mFileDescriptor == nullptr) {
        throw IOError("AbstractFileDescriptorHandler::open: "
                          "File is not opened.");
    }

    fflush(mFileDescriptor);
    fclose(mFileDescriptor);
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


} // namespace fields
} // namespace db
