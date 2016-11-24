#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {
}

TrustLinesManager::~TrustLinesManager() {
    for (auto const& iterator : mTrustLines) {
        delete iterator.second;
    }
    mTrustLines.clear();
}

byte *TrustLinesManager::serializeBytesFromTrustLineStruct(TrustLine *trustLine) {
    //Виділення 97 байт пам’яті під масив(буфер), в якому буде зберігатись сереалізований екземпляр стр-ри.
    byte *buffer = (byte *) malloc(BUCKET_SIZE);
    //Занулення байтів в буфері,
    memset(buffer, 0, BUCKET_SIZE);
    //Сереалізація значення лінії довіри до користувача в байти і запис в буфер.
    memcpy(buffer, trustAmountDataToBytes(trustLine->getIncomingTrustAmount()).data(), TRUST_AMOUNT_PART_SIZE);
    //Сереалізація значення лінії довіри від користувача в байти і запис в буфер.
    memcpy(buffer + TRUST_AMOUNT_PART_SIZE, trustAmountDataToBytes(trustLine->getOutgoingTrustAmount()).data(), TRUST_AMOUNT_PART_SIZE);
    //Сереалізація значення балансу користувача в байти і запис в буфер. Останній елемент масиву - знак балансу.
    memcpy(buffer + TRUST_AMOUNT_PART_SIZE * 2, balanceToBytes(trustLine->getBalance()).data(), BALANCE_PART_SIZE + SIGN_BYTE_PART_SIZE);
    return buffer;
}

vector<byte> TrustLinesManager::trustAmountDataToBytes(trust_amount amount) {
    vector<byte> byteSet;
    //Конвертація cpp_int шаблону в байти, результат записується у вектор.
    export_bits(static_cast<boost::multiprecision::checked_uint256_t>(amount), back_inserter(byteSet), 8);
    //Отримання розміру вектора.
    size_t filledPlace = byteSet.size();
    //Якщо значення типу cpp_int не використовує весь виділений для нього простір в пам’яті,
    //після конвертації в байти, у вектор будуть записана лише та к-сть байтів, яка являє собою значення, нульові байти відкидаються.
    //Тому, для того, щоб зберегти шаблон по якому буде визначатись зміщення, у вектор додаються нульові байти, до тих пір,
    //поки весь простір, який виділений під тип даних не буде використаний.
    for (unsigned long i = 0; i < TRUST_AMOUNT_PART_SIZE - filledPlace; ++i) {
        byteSet.push_back(0);
    }
    return byteSet;
}

vector<byte> TrustLinesManager::balanceToBytes(balance_value balance) {
    vector<byte> byteSet;
    //Конвертація cpp_int шаблону в байти, результат записується у вектор.
    export_bits(static_cast<boost::multiprecision::int256_t>(balance), back_inserter(byteSet), 8, true);
    //Отримання розміру вектора.
    size_t filledPlace = byteSet.size();
    //Якщо значення типу cpp_int не використовує весь виділений для нього простір в пам’яті,
    //після конвертації в байти, у вектор будуть записана лише та к-сть байтів, яка являє собою значення, нульові байти відкидаються.
    //Тому, для того, щоб зберегти шаблон по якому буде визначатись зміщення, у вектор додаються нульові байти, до тих пір,
    //поки весь простір, який виділений під тип даних не буде використаний.
    for (unsigned long i = 0; i < BALANCE_PART_SIZE - filledPlace; ++i) {
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

void TrustLinesManager::deserializeTrustLineStructFromBytes(byte *buffer, uuids::uuid contractorUUID) {
    //Десереалізаця байтового масиву в екземпляр структури.
    TrustLine *trustLine = new TrustLine(contractorUUID,
                                         parseTrustAmountData(buffer),
                                         parseTrustAmountData(buffer + TRUST_AMOUNT_PART_SIZE),
                                         parseBalanceData(buffer + TRUST_AMOUNT_PART_SIZE * 2));
    //Додання готового екземпляру в динамічну пам’ять.
    mTrustLines.insert(pair<boost::uuids::uuid, TrustLine *>(contractorUUID, trustLine));
}

trust_amount TrustLinesManager::parseTrustAmountData(byte *buffer) {
    trust_amount amount;
    vector<byte> bytesVector(TRUST_AMOUNT_PART_SIZE);
    vector<byte> notZeroBytesVector;
    //Копіювання даних з буфера у вектор байтів. У векторі попередньо зарезервоване місце під к-сть елементів.
    copy(buffer, buffer + TRUST_AMOUNT_PART_SIZE, bytesVector.begin());
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

balance_value TrustLinesManager::parseBalanceData(byte *buffer) {
    balance_value balance;
    vector<byte> bytesVector(BALANCE_PART_SIZE);
    vector<byte> notZeroBytesVector;
    //Отримання байту зі знаком значення.
    byte sign = buffer[BALANCE_PART_SIZE + SIGN_BYTE_PART_SIZE - 1];
    //Копіювання даних з буфера у вектор байтів. У векторі попередньо зарезервоване місце під к-сть елементів.
    copy(buffer, buffer + BALANCE_PART_SIZE, bytesVector.begin());
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
    //TODO: write bytes data in file with StorageManager, check if operation proceed success and write trust line in ram
    //Contractor's uuid is present in trust line instance, but she's not serialized in bytes array
    free(trustLineData);
    if (isTrustLineExist(trustLine->getContractorNodeUUID())) {
        map<uuids::uuid, TrustLine *>::iterator it = mTrustLines.find(trustLine->getContractorNodeUUID());
        if (it != mTrustLines.end()){
            it->second = trustLine;
        }
    } else {
        mTrustLines.insert(pair<uuids::uuid, TrustLine *>(trustLine->getContractorNodeUUID(), trustLine));
    }
}

void TrustLinesManager::removeTrustLine(const uuids::uuid contractorUUID) {
    //TODO: remove data from storage manager, check if operation proceed success and remove trust line from ram
    //TODO: something like in saveTrustLine() method
    if (isTrustLineExist(contractorUUID)) {
        delete mTrustLines.at(contractorUUID);
        mTrustLines.erase(contractorUUID);
    } else {
        throw ConflictError("Trust line to such contractor does not exist");
    }
}

bool TrustLinesManager::isTrustLineExist(const uuids::uuid contractorUUID) {
    return mTrustLines.count(contractorUUID) > 0;
}

void TrustLinesManager::open(const uuids::uuid contractorUUID, const trust_amount amount) {
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

void TrustLinesManager::close(const uuids::uuid contractorUUID) {
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

void TrustLinesManager::accept(const uuids::uuid contractorUUID, const trust_amount amount) {
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

void TrustLinesManager::reject(const uuids::uuid contractorUUID) {
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

TrustLine* TrustLinesManager::getTrustLineByContractorUUID(const uuids::uuid contractorUUID){
    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);
    }
    return nullptr;
}


