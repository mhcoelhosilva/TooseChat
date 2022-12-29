#pragma once

#include <cstdint>
#include <vector>

class DiffieHellman
{
public:
    void SetKey(uint64_t inPrivateKey);
    void SetRandomKey();
    uint64_t PowerMod(uint64_t inBase, uint64_t inPower, uint64_t inModulus);
    void CalculatePublicKey();
    void GenerateSharedSecretKey(uint64_t inPeersPublicKey);

private:
    uint64_t m_sharedPrime = 23;
    uint64_t m_sharedBase = 9;

    uint64_t m_privateKey = 0;
    uint64_t m_publicKey = 0;
    uint64_t m_peerPublicKey = 0;

    uint64_t m_sharedSecretKey = 0;
};