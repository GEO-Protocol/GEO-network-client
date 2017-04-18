#include "RecordNumbersIndex.h"


namespace db {
namespace fields {


RecordNumbersIndex::RecordNumbersIndex(
    const fs::path &path):
    AbstractFileDescriptorHandler(path) {

    try {
        open();

    } catch (Exception &e) {
        throw RuntimeError(
            "RecordNumbersIndex::RecordNumbersIndex: can't open file descriptor. Details: "
            + e.message());
    }
}

RecordNumbersIndex::FileHeader::FileHeader():
    version(1){}

RecordNumbersIndex::IndexRecord::IndexRecord(
    const RecordNumbersIndex::DataOffset offset):

    offset(offset){}

/*!
 * Throws IOError;
 */
void RecordNumbersIndex::set(
    const RecordNumber recN,
    const DataOffset storageOffset,
    const bool sync) {

    IndexRecord record(storageOffset);
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&record, sizeof(record), 1, mFileDescriptor) != 1) {
        throw IOError(
            "RecordNumbersIndex::set: can't write index record.");
    }

    if (sync)
        syncLowLevelOSBuffers();
}

/*!
 * Throws IOError;
 */
void RecordNumbersIndex::remove(
    const RecordNumber recN,
    const bool sync) {

    IndexRecord record(IndexRecord::kRemovedRecordValue);

    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&record, sizeof(record), 1, mFileDescriptor) != 1) {
        throw IOError(
            "RecordNumbersIndex::remove: "
                "can't mark record as removed.");
    }

    if (sync)
        syncLowLevelOSBuffers();
}

/*!
 * Throws NotFoundError.
 */
const RecordNumbersIndex::DataOffset RecordNumbersIndex::dataOffset(
    const RecordNumber recN) const {

    IndexRecord record(IndexRecord::kRemovedRecordValue);
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fread(&record, sizeof(record), 1, mFileDescriptor) != 1){
        throw IOError(
            "RecordNumbersIndex::dataOffset: "
                "can't read from the disk.");
    }

    if (record.offset == IndexRecord::kRemovedRecordValue ||
        record.offset == 0) {

        throw NotFoundError(
            "RecordNumbersIndex::dataOffset: no offset is associated to the recN");
    }
    return record.offset;
}

const RecordNumbersIndex::IndexRecordOffset RecordNumbersIndex::recordOffset(
    const RecordNumber recN) const {

    return (recN*sizeof(IndexRecord)) + sizeof(FileHeader);
}

/*!
 * Throws IOError in case when file header can't be read.
 * Throws ValueError in case when file version is unexpected;
 */
void RecordNumbersIndex::open() {
    AbstractFileDescriptorHandler::open();
    if (fileSize() == 0) {
        // Init default header.
        FileHeader initialHeader; // will be initialised to the defaults by the constructor.
        updateFileHeader(&initialHeader);

    } else {
        // Load data from current header.
        auto fileHeader = loadFileHeader();

        // Checking of received file version.
        // Files with unexpected file version should be rejected.
        if (fileHeader.version != 1) {
            throw ValueError(
                "RecordNumbersIndex::open: "
                    "unexpected file version occurred.");
        }
    }
}

/*!
 * Throws IOError;
 */
RecordNumbersIndex::FileHeader RecordNumbersIndex::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "RecordNumbersIndex::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}

/*!
 * Atomically updates file header.
 * Throws IOError in case when write operation failed.
 */
void RecordNumbersIndex::updateFileHeader(
    const FileHeader *header) const {

    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(FileHeader), 1, mFileDescriptor) != 1) {
        throw IOError(
            "RecordNumbersIndex::updateFileHeader: "
                "can't write header to the disk.");
    }
    if (fsync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "RecordNumbersIndex::updateFileHeader: "
                "can't sync buffers with the disk.");
    }
}

void RecordNumbersIndex::commit() {
    syncLowLevelOSBuffers();
}


} // namespace fields
} // namespace db


