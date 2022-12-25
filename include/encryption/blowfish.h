#pragma once

#include "encryption.h"

class Blowfish : public Encryption
{
public:
    Blowfish(const std::string& inKey);

    void encrypt(char* outDst, const char* inSrc, int inByteLength) override;
    void decrypt(char* outDst, const char* inSrc, int inByteLength) override;

private:
    uint32_t f(uint32_t x);
    void blowfishEncrypt(uint32_t* L, uint32_t* R);
    void blowfishDecrypt(uint32_t* L, uint32_t* R);

    uint32_t m_P[18];
    uint32_t m_S[4][256];
};