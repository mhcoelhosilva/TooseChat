#include "encryption/diffiehellman.h"

#include <random>

void DiffieHellman::SetKey(uint64_t inPrivateKey)
{
    m_privateKey = inPrivateKey;

    CalculatePublicKey();
}

void DiffieHellman::SetRandomKey()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, m_sharedPrime - 1);

    m_privateKey = distr(gen);

    CalculatePublicKey();
}

// calculate (inBase ^ inPower) mod(inModulus)
uint64_t DiffieHellman::PowerMod(uint64_t inBase, uint64_t inPower, uint64_t inModulus)
{
    if (inPower == 1)
        return inBase;
 
    return (((uint64_t)pow(inBase, inPower)) % inModulus);
}
void DiffieHellman::CalculatePublicKey()
{
    m_publicKey = PowerMod(m_sharedBase, m_privateKey, m_sharedPrime);
}

void DiffieHellman::GenerateSharedSecretKey(uint64_t inPeersPublicKey)
{
    m_sharedSecretKey = PowerMod(inPeersPublicKey, m_privateKey, m_sharedPrime);
}