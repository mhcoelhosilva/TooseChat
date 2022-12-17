import os
import subprocess

class TooseChat:
    def __init__(self, hostName, port):
        self.hostName = hostName
        self.port = port
        self.processes = list()

    def rebuild(self):
        return

    def initialize(self):
        os.chdir("Build")
        return

    def start_client(self):
        self.processes.append(subprocess.Popen(['CppChat.exe', '-c', self.hostName, '-p', self.port],
                        stdout=subprocess.PIPE, 
                        stderr=subprocess.PIPE))

        return

    def start_server(self):
        self.processes.append(subprocess.Popen(['CppChat.exe', '-h', '-p', self.port],
                        stdout=subprocess.PIPE, 
                        stderr=subprocess.PIPE))
        
        return

    def wait_for_processes(self):
        while len(self.processes) > 0:
            for i in range(len(self.processes) - 1, -1, -1):
                p = self.processes[i]
                output = p.stdout.readline()
                print(output.strip())
                
                return_code = p.poll()
                if return_code is not None:
                    print('RETURN CODE', return_code)
                    # Process has finished, read rest of the output 
                    for output in p.stdout.readlines():
                        print(output.strip())
                    
                    del self.processes[i]
                    
        return

def main():
    chat = TooseChat('localhost', '12')
    chat.initialize()
    chat.start_server()
    chat.start_client()
    chat.wait_for_processes()

if __name__ == "__main__":
    main()