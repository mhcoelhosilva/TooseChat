#include "utils.h"

#include <random>
#include <algorithm>
#include <iostream>

namespace Utils
{
    // --- AsyncGetline ---
    // credit: https://stackoverflow.com/users/1599699/andrew

    //AsyncGetline is a class that allows for asynchronous CLI getline-style input
    //(with 0% CPU usage!), which normal iostream usage does not easily allow.
    AsyncGetline::AsyncGetline()
    {
        input = "";
        sendOverNextLine = true;
        continueGettingInput = true;

        //Start a new detached thread to call getline over and over again and retrieve new input to be processed.
        thread([&]()
        {
            //Non-synchronized string of input for the getline calls.
            string synchronousInput;
            char nextCharacter;

            //Get the asynchronous input lines.
            do
            {
                //Start with an empty line.
                synchronousInput = "";

                //Process input characters one at a time asynchronously, until a new line character is reached.
                while (continueGettingInput)
                {
                    //See if there are any input characters available (asynchronously).
                    while (cin.peek() == EOF)
                    {
                        //Ensure that the other thread is always yielded to when necessary. Don't sleep here;
                        //only yield, in order to ensure that processing will be as responsive as possible.
                        this_thread::yield();
                    }

                    //Get the next character that is known to be available.
                    nextCharacter = cin.get();

                    //Check for new line character.
                    if (nextCharacter == '\n')
                    {
                        break;
                    }

                    //Since this character is not a new line character, add it to the synchronousInput string.
                    synchronousInput += nextCharacter;
                }

                //Be ready to stop retrieving input at any moment.
                if (!continueGettingInput)
                {
                    break;
                }

                //Wait until the processing thread is ready to process the next line.
                while (continueGettingInput && !sendOverNextLine)
                {
                    //Ensure that the other thread is always yielded to when necessary. Don't sleep here;
                    //only yield, in order to ensure that the processing will be as responsive as possible.
                    this_thread::yield();
                }

                //Be ready to stop retrieving input at any moment.
                if (!continueGettingInput)
                {
                    break;
                }

                //Safely send the next line of input over for usage in the processing thread.
                inputLock.lock();
                input = synchronousInput;
                inputLock.unlock();

                //Signal that although this thread will read in the next line,
                //it will not send it over until the processing thread is ready.
                sendOverNextLine = false;
            }
            while (continueGettingInput && input != "exit");
        }).detach();
    }

    //Stop getting asynchronous CLI input.
    AsyncGetline::~AsyncGetline()
    {
        //Stop the getline thread.
        continueGettingInput = false;
    }

    //Get the next line of input if there is any; if not, sleep for a millisecond and return an empty string.
    string AsyncGetline::getLine()
    {
        //See if the next line of input, if any, is ready to be processed.
        string result = "";
        if (sendOverNextLine)
        {
            //Don't consume the CPU while waiting for input; this_thread::yield()
            //would still consume a lot of CPU, so sleep must be used.
            //this_thread::sleep_for(chrono::milliseconds(1));

            result = "";
        }
        else
        {
            //Retrieve the next line of input from the getline thread and store it for return.
            inputLock.lock();
            string returnInput = input;
            inputLock.unlock();

            //Also, signal to the getline thread that it can continue
            //sending over the next line of input, if available.
            sendOverNextLine = true;

            result = returnInput;
        }

        return result;
    }

    // --- PrimeNumberGenerator ---

    void PrimeNumberGenerator::GeneratePrimes(uint64_t inSearchUntil)
    {
        // Using Sieve of Eratosthenes (https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)
        // O(n log log n)

        if (m_generatedPrimes && m_searchedUntil >= inSearchUntil)
            return;

        uint64_t n = inSearchUntil;

        // A represents ints 2 through n
        std::vector<bool> A(n - 1, true);

        for (uint64_t i = 2; i * i < n; ++i)
            if (A[i - 2] == true)
                for (uint64_t j = i * i; j < n; j += i)
                    A[j - 2] = false;

        m_primes.clear();
        for (uint64_t i = 0; i < A.size(); ++i)
            if (A[i] == true)
                m_primes.push_back(i + 2);

        m_generatedPrimes = true;
        m_searchedUntil = inSearchUntil;
    }

    uint64_t PrimeNumberGenerator::GetRandomPrimeNumber(uint64_t inEndRange /*= 100*/)
    {
        GeneratePrimes(inEndRange);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, m_primes.size() - 1);

        int randomIndex = distr(gen);
        if (randomIndex < 0 || randomIndex >= m_primes.size())
        {
            std::cerr << "Random index " << randomIndex;
            std::cerr << " out of m_primes range (n = " << m_primes.size() << ")." << std::endl;
            return -1;
        }

        return m_primes[randomIndex];
    }
};