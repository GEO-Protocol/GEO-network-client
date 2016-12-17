#include "RecordNumbersIndex.h"


namespace db {
namespace fields {

RecordNumbersIndex::RecordNumbersIndex(const char *path, const char *filename) :

    AbstractFileDescriptorHandler(path, filename) {
    open(kWriteAccessMode);
}

RecordNumbersIndex::FileHeader::FileHeader():
    version(1){}

RecordNumbersIndex::IndexRecord::IndexRecord() {}

RecordNumbersIndex::IndexRecord::IndexRecord(
    const RecordNumbersIndex::DataOffset offset):

    offset(offset){}

/*!
 * Updates data offset of the record with number == "recN".
 * Throws IOError in case if operation can't be finished successfully.
 *
 * @param recN - number of the record that should be updated.
 * @param storageOffset - data offset into the external storage.
 * @param flushBuffers - specifies if fdatasync() should be called
 *      after successfull operation.
 */
void RecordNumbersIndex::set(
    const RecordNumber recN,
    const DataOffset storageOffset) {

    IndexRecord record(storageOffset);

    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&record, sizeof(record), 1, mFileDescriptor) != 1) {
        throw IOError(
            "RecordNumbersIndex::set: "
                "can't write index record.");
    }
}

/*!
 * Marks record with record number "recN" as removed.
 * Records are never physically removing from index.
 *
 * Throws IOError in case if operation can't be finished successfully.
 *
 * @param recN - number of the record that should be removed.
 * @param flushBuffers - specifies if fdatasync() should be called
 *      after successfull operation.
 */
void RecordNumbersIndex::remove(
    const RecordNumber recN) {

    IndexRecord record(IndexRecord::kRemovedRecordValue);

    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&record, sizeof(record), 1, mFileDescriptor) != 1) {
        throw IOError(
            "RecordNumbersIndex::remove: "
                "can't mark record as removed.");
    }
}

/*!
 * @param recN - number of the record that should be read.
 * @return offset of the data block that is associated with "recN"
 */
const RecordNumbersIndex::DataOffset RecordNumbersIndex::dataOffset(
    const RecordNumber recN) const {

    IndexRecord record;
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fread(&record, sizeof(record), 1, mFileDescriptor) != 1){
        throw IOError(
            "RecordNumbersIndex::dataOffset: "
                "can't read from the disk.");
    }

    return record.offset;
}

const RecordNumbersIndex::IndexRecordOffset RecordNumbersIndex::recordOffset(
    const RecordNumber recN) const {

    return (recN*sizeof(IndexRecord)) + sizeof(FileHeader);
}

void RecordNumbersIndex::open(
    const char *accessMode) {

    AbstractFileDescriptorHandler::open(accessMode);

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
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "RecordNumbersIndex::updateFileHeader: "
                "can't sync buffers with the disk.");
    }
}
} // namespace fields
} // namespace db


