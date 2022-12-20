#include <string>

class Encryption
{
public:
    virtual void encrypt(char* outDst, const char* inSrc, int inByteLength) = 0;
    virtual void decrypt(char* outDst, const char* inSrc, int inByteLength) = 0;
protected:
    std::string m_key;
};