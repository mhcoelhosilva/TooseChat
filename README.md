# TooseChat

This is an experiment in making a basic chat application in C++ using Winsock. The goal is to learn a bit about networking and encryption along the way.

Here's the feature wishlist:

+ Basic command-line interface
+ Client-to-client messaging
+ Message encryption
+ Automated performance testing in Python

And a few stretch goals:

+ GUI using ImGui
+ Linux support

## Basic Build Instructions

In the main project directory, create a 'build' directory and enter it. Then, to configure the build in the shell:

'cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug'

or

'cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release'

Then build with CMake. The project has been set up for VSCode.

## Basic Usage Instructions

Running 'TooseChat.exe --help' from the commandline will give you the following usage information:

'''
Usage:
TooseChat.exe [options]

Options:
 --client <host-name>        = Run as client.
 --server                    = Run as server.
 --port <port-number>        = Port number to use.
 --key <encryption-key>      = Encryption key to use.
 --help                      = See usage instructions.
'''

The basic use case for testing is to run one local instance of the server with 'TooseChat.exe' or 'TooseChat.exe -s' (default port) and one local instance of the client with 'TooseChat.exe -c localhost' (default port and encryption key).