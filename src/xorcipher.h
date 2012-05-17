#ifndef XORCIPHER_H
#define XORCIPHER_H
#include <QString>

// simple Xor encryption/decryption
class XorCipher
{
private:
    XorCipher();
public:
    // todo also encrypt the key if it is kept in memory
    static QString encrypt(QString sourceString, int key);
    static QString decrypt(QString encryptedString, int key);


};

#endif // XORCIPHER_H
