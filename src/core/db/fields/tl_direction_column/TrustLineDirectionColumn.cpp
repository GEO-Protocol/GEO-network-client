#include "TrustLineDirectionColumn.h"


namespace db {
namespace fields {


TrustLineDirectionColumn::TrustLineDirectionColumn(
    const fs::path &path):
    AbstractFileDescriptorHandler(path / fs::path("data.bin")) {

    open();
}


void TrustLineDirectionColumn::set(
    const db::AbstractRecordsHandler::RecordNumber recN,
    const TrustLineDirection direction,
    const bool sync) {

    // Writing record number to the disk index.
    SerializedTrustLineDirection serializableDirection(direction);
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1 &&
        fwrite(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::set: can't write record.");
    }

    if (sync) {
        syncLowLevelOSBuffers();
    }

    // Updating memory index
    switch (direction) {
        case Incoming: {
            mIncomingRecordNumbers.insert(recN);
            break;
        }
        case Outgoing: {
            mOutoingRecordNumbers.insert(recN);
            break;
        }
        case Both: {
            mBothDirectedRecordNumbers.insert(recN);
            break;
        }
        case Nowhere: {
            throw ValueError(
                "TrustLineDirectionColumn::set: unexpected direction received.");
        }
    }
}

void TrustLineDirectionColumn::remove(
    const db::AbstractRecordsHandler::RecordNumber recN,
    const bool sync) {

    try {
        // Updating memory index
        switch (direction(recN)) {
            case Incoming: {
                mIncomingRecordNumbers.erase(recN);
                break;
            }
            case Outgoing: {
                mOutoingRecordNumbers.erase(recN);
                break;
            }
            case Both: {
                mBothDirectedRecordNumbers.erase(recN);
                break;
            }
            case Nowhere: {
                break;
            }
        }

    } catch (NotFoundError &){
        // direction(recN) may throw NotFound in case when recN is absent in index.
        // In this case - no one memory index contains the value.
    }


    const SerializedTrustLineDirection removedRecordValue = Nowhere;
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&removedRecordValue, sizeof(removedRecordValue), 1, mFileDescriptor) != 1 &&
        fwrite(&removedRecordValue, sizeof(removedRecordValue), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::set: can't remove record.");
    }

    if (sync) {
        syncLowLevelOSBuffers();
    }
}

/*!
 * @param recN - number of the record that should be returned.
 * @return direction of the trust line;
 *
 *
 * Throws NotFoundError in case when no recN is in index.
 */
const TrustLineDirection TrustLineDirectionColumn::direction(
    const db::AbstractRecordsHandler::RecordNumber recN) const {

    SerializedTrustLineDirection serializableDirection;
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fread(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1 &&
        fread(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::set: can't read record.");
    }

    if (serializableDirection == Nowhere) {
        throw NotFoundError(
            "TrustLineDirectionColumn::direction: no record associated with recN");
    } else {
        return TrustLineDirection(serializableDirection);
    }
}



TrustLineDirectionColumn::FileHeader::FileHeader():
    version(1){}


void TrustLineDirectionColumn::open() {
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
                "TrustLineDirectionColumn::open: "
                    "unexpected file version occurred.");
        }

        loadIndexIntoMemory();
    }
}

TrustLineDirectionColumn::FileHeader TrustLineDirectionColumn::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}
void TrustLineDirectionColumn::updateFileHeader(
    const TrustLineDirectionColumn::FileHeader *header) const {

    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(FileHeader), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::updateFileHeader: "
                "can't write header to the disk.");
    }
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "TrustLineDirectionColumn::updateFileHeader: "
                "can't sync buffers with the disk.");

    }
}

const TrustLineDirectionColumn::IndexRecordOffset
TrustLineDirectionColumn::recordOffset(const AbstractRecordsHandler::RecordNumber recN) const {
    return (recN * sizeof(SerializedTrustLineDirection)) + sizeof(FileHeader);
}

const flat_set<
    AbstractRecordsHandler::RecordNumber,
    less<AbstractRecordsHandler::RecordNumber>,
    new_allocator<AbstractRecordsHandler::RecordNumber>> &
TrustLineDirectionColumn::incomingDirectedRecordsNumbers() const {
    return mIncomingRecordNumbers;
}

const flat_set<
    AbstractRecordsHandler::RecordNumber,
    less<AbstractRecordsHandler::RecordNumber>,
    new_allocator<AbstractRecordsHandler::RecordNumber>> &
TrustLineDirectionColumn::outgoingDirectedRecordsNumbers() const {
    return mOutoingRecordNumbers;
}

const flat_set<
    AbstractRecordsHandler::RecordNumber,
    less<AbstractRecordsHandler::RecordNumber>,
    new_allocator<AbstractRecordsHandler::RecordNumber>> &
TrustLineDirectionColumn::bothDirectedRecordsNumbers() const {
    return mBothDirectedRecordNumbers;
}

void TrustLineDirectionColumn::loadIndexIntoMemory() {
    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);

    size_t readingWindow = 512;
    SerializedTrustLineDirection directions[readingWindow];
    RecordNumber currentRecordNumber = 0;

    while (true) {
        size_t totalRead = fread(&directions, sizeof(SerializedTrustLineDirection), readingWindow, mFileDescriptor);
        if (totalRead == 0) {
            if (feof(mFileDescriptor)){
                break;
            }

            throw IOError(
                "TrustLineDirectionColumn::loadIndexIntoMemory: "
                    "can't read from the disk.");
        }

        for (size_t i=0; i<totalRead; ++i){
            // Note:
            // flat_set uses binary search algorithm to find position for the next element inserting,
            // and this algorithm has a special optimizations for inserting of the value,
            // that is bigger than all previous values.
            //
            // Record numbers are constantly increasing on each iteration,
            // so it is OK to call insert() without performance harm.

            switch (directions[i]) {
                case Incoming: {
                    mIncomingRecordNumbers.insert(currentRecordNumber);
                    break;
                }
                case Outgoing: {
                    mOutoingRecordNumbers.insert(currentRecordNumber);
                    break;
                }
                case Both: {
                    mBothDirectedRecordNumbers.insert(currentRecordNumber);
                    break;
                }
                case Nowhere: {
                    break;
                }
            }

            currentRecordNumber++;
        }
    }
}

void TrustLineDirectionColumn::commit() {
    syncLowLevelOSBuffers();
}


}
}

