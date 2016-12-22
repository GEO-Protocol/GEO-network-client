#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {
    mTrustLinesStorage = new TrustLinesStorage("trust_lines_storage.bin");
    getTrustLinesFromStorage();
}

TrustLinesManager::~TrustLinesManager() {
    for (auto const& iterator : mTrustLines) {
        delete iterator.second;
    }
    mTrustLines.clear();
    delete mTrustLinesStorage;
}

byte *TrustLinesManager::serializeBytesFromTrustLineStruct(TrustLine *trustLine) {
    //Виділення 97 байт пам’яті під масив(буфер), в якому буде зберігатись сереалізований екземпляр стр-ри.
    byte *buffer = (byte *) malloc(kBucketSize);
    //Занулення байтів в буфері,
    memset(buffer, 0, kBucketSize);
    //Сереалізація значення лінії довіри до користувача в байти і запис в буфер.
    memcpy(buffer, trustAmountDataToBytes(trustLine->getIncomingTrustAmount()).data(), kTrustAmountPartSize);
    //Сереалізація значення лінії довіри від користувача в байти і запис в буфер.
    memcpy(buffer + kTrustAmountPartSize, trustAmountDataToBytes(trustLine->getOutgoingTrustAmount()).data(), kTrustAmountPartSize);
    //Сереалізація значення балансу користувача в байти і запис в буфер. Останній елемент масиву - знак балансу.
    memcpy(buffer + kTrustAmountPartSize * 2, balanceToBytes(trustLine->getBalance()).data(), kBalancePartSize + kSignBytePartSize);
    return buffer;
}

vector<byte> TrustLinesManager::trustAmountDataToBytes(const trust_amount &amount) {
    vector<byte> byteSet;
    //Конвертація cpp_int шаблону в байти, результат записується у вектор.
    export_bits(static_cast<boost::multiprecision::checked_uint256_t>(amount), back_inserter(byteSet), 8);
    //Отримання розміру вектора.
    size_t filledPlace = byteSet.size();
    //Якщо значення типу cpp_int не використовує весь виділений для нього простір в пам’яті,
    //після конвертації в байти, у вектор будуть записана лише та к-сть байтів, яка являє собою значення, нульові байти відкидаються.
    //Тому, для того, щоб зберегти шаблон по якому буде визначатись зміщення, у вектор додаються нульові байти, до тих пір,
    //поки весь простір, який виділений під тип даних не буде використаний.
    for (unsigned long i = 0; i < kTrustAmountPartSize - filledPlace; ++i) {
        byteSet.push_back(0);
    }
    return byteSet;
}

vector<byte> TrustLinesManager::balanceToBytes(const balance_value &balance) {
    vector<byte> byteSet;
    //Конвертація cpp_int шаблону в байти, результат записується у вектор.
    export_bits(static_cast<boost::multiprecision::int256_t>(balance), back_inserter(byteSet), 8, true);
    //Отримання розміру вектора.
    size_t filledPlace = byteSet.size();
    //Якщо значення типу cpp_int не використовує весь виділений для нього простір в пам’яті,
    //після конвертації в байти, у вектор будуть записана лише та к-сть байтів, яка являє собою значення, нульові байти відкидаються.
    //Тому, для того, щоб зберегти шаблон по якому буде визначатись зміщення, у вектор додаються нульові байти, до тих пір,
    //поки весь простір, який виділений під тип даних не буде використаний.
    for (unsigned long i = 0; i < kBalancePartSize - filledPlace; ++i) {
        byteSet.push_back(0);
    }
    //Перевірка знаку:
    //якщо число негативне -
    if (balance.sign() == -1) {
        //в контрольний байт пишеться 1.
        byteSet.push_back(1);
    } else {
        //інакше, якщо значення позитивне - в контрольний байт пишеться 0
        byteSet.push_back(0);
    }
    return byteSet;
}

void TrustLinesManager::deserializeTrustLineStructFromBytes(const byte *buffer, const NodeUUID &contractorUUID) {
    //Десереалізаця байтового масиву в екземпляр структури.
    TrustLine *trustLine = new TrustLine(contractorUUID,
                                         parseTrustAmountData(buffer),
                                         parseTrustAmountData(buffer + kTrustAmountPartSize),
                                         parseBalanceData(buffer + kTrustAmountPartSize * 2));
    //Додання готового екземпляру в динамічну пам’ять.
    mTrustLines.insert(pair<NodeUUID, TrustLine *>(contractorUUID, trustLine));
}

trust_amount TrustLinesManager::parseTrustAmountData(const byte *buffer) {
    trust_amount amount;
    vector<byte> bytesVector(kTrustAmountPartSize);
    vector<byte> notZeroBytesVector;
    //Копіювання даних з буфера у вектор байтів. У векторі попередньо зарезервоване місце під к-сть елементів.
    copy(buffer, buffer + kTrustAmountPartSize, bytesVector.begin());
    //Отримання не нульових байтів і копіювання їх у вектор не нуьлових байтів
    for (unsigned long i = 0; i < bytesVector.size(); ++i) {
        if (bytesVector.at(i) != 0) {
            notZeroBytesVector.push_back(bytesVector.at(i));
        }
    }
    //Якщо значення не рівне нулю, тоді у векторі не нульових байтів будуть якісь дані -
    if (notZeroBytesVector.size() > 0) {
        //конвертація байтів в cpp_int шаблон з вектору не нульових байтів.
        import_bits(amount, notZeroBytesVector.begin(), notZeroBytesVector.end());
    } else if (notZeroBytesVector.size() == 0) {
        //конвертація байтів в cpp_int шаблон з вектору байтів.
        import_bits(amount, bytesVector.begin(), bytesVector.end());
    }
    return amount;
}

balance_value TrustLinesManager::parseBalanceData(const byte *buffer) {
    balance_value balance;
    vector<byte> bytesVector(kBalancePartSize);
    vector<byte> notZeroBytesVector;
    //Отримання байту зі знаком значення.
    byte sign = buffer[kBalancePartSize + kSignBytePartSize - 1];
    //Копіювання даних з буфера у вектор байтів. У векторі попередньо зарезервоване місце під к-сть елементів.
    copy(buffer, buffer + kBalancePartSize, bytesVector.begin());
    //Отримання не нульових байтів і копіювання їх у вектор не нуьлових байтів
    for (unsigned long i = 0; i < bytesVector.size(); ++i) {
        if (bytesVector.at(i) != 0) {
            notZeroBytesVector.push_back(bytesVector.at(i));
        }
    }
    //Якщо значення не рівне нулю, тоді у векторі не нульових байтів будуть якісь дані -
    if (notZeroBytesVector.size() > 0) {
        //конвертація байтів в cpp_int шаблон з вектору не нульових байтів.
        import_bits(balance, notZeroBytesVector.begin(), notZeroBytesVector.end(), 8, true);
    } else if (notZeroBytesVector.size() == 0) {
        //конвертація байтів в cpp_int шаблон з вектору байтів.
        import_bits(balance, bytesVector.begin(), bytesVector.end(), 8, true);
    }
    //Якщо байт знаку рівний одиниці -
    if (sign == 1) {
        //реверс значення у негативне.
        balance = balance * -1;
    }
    return balance;
}

void TrustLinesManager::saveTrustLine(TrustLine *trustLine) {
    byte *trustLineData = serializeBytesFromTrustLineStruct(trustLine);
    if (isTrustLineExist(trustLine->getContractorNodeUUID())) {
        try{
            mTrustLinesStorage->modifyExistingTrustLineInStorage(trustLine->getContractorNodeUUID(), trustLineData, kBucketSize);
        }catch (std::exception &e){
            throw IOError(string(string("Can't store existing trust line in file. Message-> ") + string(e.what())).c_str());
        }
        map<NodeUUID, TrustLine *>::iterator it = mTrustLines.find(trustLine->getContractorNodeUUID());
        if (it != mTrustLines.end()){
            it->second = trustLine;
        }
    } else {
        try{
            mTrustLinesStorage->writeNewTrustLineInStorage(trustLine->getContractorNodeUUID(), trustLineData, kBucketSize);
        }catch (std::exception &e){
            throw IOError(string(string("Can't store new trust line in file. Message-> ") + string(e.what())).c_str());
        }
        mTrustLines.insert(pair<NodeUUID, TrustLine *>(trustLine->getContractorNodeUUID(), trustLine));
    }
    free(trustLineData);
}

void TrustLinesManager::removeTrustLine(const NodeUUID &contractorUUID) {
    if (isTrustLineExist(contractorUUID)) {
        try{
            mTrustLinesStorage->removeTrustLineFromStorage(contractorUUID);
        }catch (std::exception &e){
            throw IOError(string(string("Can't remove trust line from file. Message-> ") + string(e.what())).c_str());
        }
        delete mTrustLines.at(contractorUUID);
        mTrustLines.erase(contractorUUID);
    } else {
        throw ConflictError("Trust line to such contractor does not exist");
    }
}

bool TrustLinesManager::isTrustLineExist(const NodeUUID &contractorUUID) {
    return mTrustLines.count(contractorUUID) > 0;
}

void TrustLinesManager::getTrustLinesFromStorage() {
    vector<NodeUUID> contractorsUUIDs = mTrustLinesStorage->getAllContractorsUUIDs();
    if (contractorsUUIDs.size() > 0){
        for (auto const &item : contractorsUUIDs){
            const storage::Block *block = mTrustLinesStorage->readTrustLineFromStorage(item);
            deserializeTrustLineStructFromBytes(block->data(), item);
            delete block;
        }
    }
}

void TrustLinesManager::open(const NodeUUID &contractorUUID, const trust_amount &amount) {
    //Якщо значення лінії довіри, що відкривається більше за нуль
    if (amount > ZERO_CHECKED_INT256_VALUE){
        //Якщо існує вхідна лінія довіри від контрагента
        if (isTrustLineExist(contractorUUID)) {
            //Отримання вказівника на екземпляр
            TrustLine *trustLine = mTrustLines.at(contractorUUID);
            //Якщо вихідна лінія рівна нулю, тобто лінія довіри від користувача до контрагента раніше не була відкрита
            if (trustLine->getOutgoingTrustAmount() == 0) {
                //Встановлення розміру вихідної лінії довіри
                //Збереження лінії довіри
                trustLine->setOutgoingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine), amount);
            } else {
                //Інакше відкрити лінію не можливо, вихідна лінія довіри вже існує
                throw ConflictError("Сan not open outgoing trust line. Outgoing trust line to such contractor already exist.");
            }
        } else {
            //Інакше, створення нового екзмепляру лінії довіри
            //Встановлення розміру вихідної лінії довіри
            TrustLine *trustLine = new TrustLine(contractorUUID, 0, amount, 0);
            //Збереження лінії довіри
            saveTrustLine(trustLine);
        }
    } else {
        //Інакше відкрити лінію не можливо, значення лінії менше/рівне нулю
        throw ValueError("Сan not open outgoing trust line. Outgoing trust line amount less or equals to zero.");
    }
}

void TrustLinesManager::close(const NodeUUID &contractorUUID) {
    //Якщо лінія довіри по відношенню до контрагента існує
    if (isTrustLineExist(contractorUUID)) {
        TrustLine *trustLine = mTrustLines.at(contractorUUID);
        //Якщо користувач довіряє контрагенту, вихідна лінія довіри більша за нуль
        if (trustLine->getOutgoingTrustAmount() > ZERO_CHECKED_INT256_VALUE){
            //Якщо баланс менший або рівний нулю (лінія довіри від користувача не використана)
            if (trustLine->getBalance() <= ZERO_INT256_VALUE) {
                //Якщо лінія довіри до користувача рівна нулю
                if (trustLine->getIncomingTrustAmount() == ZERO_CHECKED_INT256_VALUE) {
                    //Вилучення лінії довіри цілком
                    trustLine = nullptr;
                    removeTrustLine(contractorUUID);
                } else {
                    //Інакше, якщо лінія довіри до користувача існує, редагування значення вихідної лінії довіри
                    trustLine->setOutgoingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine), 0);
                }
            } else {
                //Інакше лінію довіри до контрагента закрити не можливо, через помилку балансу, баланс більший за нуль, контрагент
                //використав лінію довіри від користувача
                throw PreconditionFaultError("Сan not close outgoing trust line. Contractor already used part of amount.");
            }
        } else {
            //Інакше лінію довіри до контрагента закрити не можливо, значення вихідної лінії довіри менше/рівне нулю
            throw ValueError("Сan not close outgoing trust line. Outgoing trust line amount less or equals to zero.");
        }
    } else {
        //Інакше лінію довіри до контрагента закрити не можливо, лінія довіри по відношенню до контрагента не існує
        throw ConflictError("Сan not close outgoing trust line. Trust line to such contractor does not exist.");
    }
}

void TrustLinesManager::accept(const NodeUUID &contractorUUID, const trust_amount &amount) {
    //Якщо значення лінії довіри, що приймається більше за нуль
    if (amount > ZERO_CHECKED_INT256_VALUE){
        //Якщо існує вихідна лінія довіри до контрагента
        if (isTrustLineExist(contractorUUID)) {
            //Отримання вказівника на екземпляр
            TrustLine *trustLine = mTrustLines.at(contractorUUID);
            //Якщо вхідна лінія рівна нулю, тобто лінія довіри від контрагента до користувача раніше не була відкрита
            if (trustLine->getIncomingTrustAmount() == 0){
                //Встановлення розміру вхідної лінії довіри
                //Збереження лінії довіри
                trustLine->setIncomingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine), amount);
            } else {
                //Інакше прийняти лінію довіри не можливо, вхідна лінія довіри вже існує
                throw ConflictError("Сan not accept incoming trust line. Incoming trust line to such contractor already exist.");
            }
        } else {
            //Інакше, створення нового екзмепляру лінії довіри
            //Встановлення розміру вхідної лінії довіри
            TrustLine *trustLine = new TrustLine(contractorUUID, amount, 0, 0);
            //Збереження лінії довіри
            saveTrustLine(trustLine);
        }
    } else {
        //Інакше прийняти лінію довіри не можливо, значення лінії менше/рівне нулю
        throw ValueError("Сan not accept incoming trust line. Incoming trust line amount less or equals to zero.");
    }
}

void TrustLinesManager::reject(const NodeUUID &contractorUUID) {
    //Якщо лінія довіри по відношенню до контрагента існує
    if (isTrustLineExist(contractorUUID)) {
        TrustLine *trustLine = mTrustLines.at(contractorUUID);
        //Якщо контрагент довіряє користувачу, вхідна лінія довіри більша за нуль
        if (trustLine->getIncomingTrustAmount() > ZERO_CHECKED_INT256_VALUE){
            //Якщо баланс більший або рівний нулю (лінія довіри від контрагента не використана)
            if (trustLine->getBalance() >= ZERO_INT256_VALUE) {
                //Якщо лінія довіри до контрагента рівна нулю
                if (trustLine->getOutgoingTrustAmount() == ZERO_CHECKED_INT256_VALUE) {
                    //Вилучення лінії довіри цілком
                    trustLine = nullptr;
                    removeTrustLine(contractorUUID);
                } else {
                    //Інакше, якщо лінія довіри до контрагента існує, редагування значення вхідної лінії довіри
                    trustLine->setIncomingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine), 0);
                }
            } else {
                //Інакше лінію довіри від контрагента відхилити не можливо, через помилку балансу, баланс менший за нуль, користувач
                //використав лінію довіри від контрагента
                throw PreconditionFaultError("Сan not reject incoming trust line. User already used part of amount.");
            }
        } else {
            //Інакше лінію довіри від контрагента відхилити не можливо, значення вхідної лінії довіри менше/рівне нулю
            throw ValueError("Сan not reject incoming trust line. Incoming trust line amount less or equals to zero.");
        }
    } else {
        //Інакше лінію довіри від контрагента відхилити не можливо, лінія довіри по відношенню до контрагента не існує
        throw ConflictError("Сan not reject incoming trust line. Trust line to such contractor does not exist.");
    }
}

TrustLine* TrustLinesManager::getTrustLineByContractorUUID(const NodeUUID &contractorUUID){
    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);
    } else {
        throw ConflictError("Can't find trust line by such contractor UUID.");
    }
}


