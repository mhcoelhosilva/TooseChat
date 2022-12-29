#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstdlib>

namespace Utils
{
    using namespace std;

    // --- AsyncGetline ---
    // credit: https://stackoverflow.com/users/1599699/andrew

    class AsyncGetline
    {
    public:
        AsyncGetline();
        ~AsyncGetline();

        string getLine();

    private:
        //Cross-thread-safe boolean to tell the getline thread to stop when AsyncGetline is deconstructed.
        atomic<bool> continueGettingInput;

        //Cross-thread-safe boolean to denote when the processing thread is ready for the next input line.
        //This exists to prevent any previous line(s) from being overwritten by new input lines without
        //using a queue by only processing further getline input when the processing thread is ready.
        atomic<bool> sendOverNextLine;

        //Mutex lock to ensure only one thread (processing vs. getline) is accessing the input string at a time.
        mutex inputLock;

        //string utilized safely by each thread due to the inputLock mutex.
        string input;
    };

    // --- PrimeNumberGenerator ---

    class PrimeNumberGenerator
    {
    public:
        uint64_t GetRandomPrimeNumber(uint64_t inRangeEnd = 100);

    private:
        void GeneratePrimes(uint64_t inSearchUntil);

        vector<uint64_t> m_primes;
        bool m_generatedPrimes = false;
        uint64_t m_searchedUntil = 0;
    };
};