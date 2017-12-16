#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <wait.h>
#include <cstring>
class TShell
{
    protected:
        bool _running;
        char * const*envp;
        std::istream &in;
        std::ostream &out;
        std::string lineCache;
        std::string seg;
        std::stringstream lineStream;

        void _cd()
        {
            lineStream >> seg;
            if (!lineStream)
            {
                out << "Error! You must provide an argument to `cd`" << std::endl;
                return;
            }
            int result = chdir(seg.c_str());
            if (0 == result) return;
            // error
            out << "Error while changing working directory! Code(" << result << ")";
        }
    public:
        TShell(std::istream &_in, std::ostream &_out, char * const*_envp):in(_in),out(_out),_running(true), envp(_envp) {}
        bool running()
        {
            return _running;
        }
        void printPS()
        {
            static char buf[256];
            getcwd(buf, 256);
            out << std::flush << buf << "> ";
        }
        void readLine()
        {
            getline(in, lineCache);
            lineStream.clear();
            lineStream.str(lineCache);
        }
        void exec()
        {
            seg = "";
            lineStream >> seg;
            // Empty stream
            if (!lineStream || seg.length() == 0) return;
            // Comment line
            if (seg.at(0) == '#') return;
            if (seg == "exit")
            {
                _running = false;
                return;
            }
            if (seg == "cd")
            {
                _cd();
                return;
            }
            // outside command
            // out << "Execute: " << lineCache;
            bool wait = true;
            if (lineCache.at(lineCache.length() - 1) == '&')
            {
                wait = false;
                lineCache.erase(lineCache.end() - 1);
            }
            pid_t pid = vfork();
            if (pid < 0)
            {
                out << "Error while spawning child process!";
                return;
            }
            if (pid == 0)
            {
                // child
                char *argv[64];
                lineStream.clear();
                lineStream.str(lineCache);
                int i = 0;
                while (lineStream)
                {
                    seg == "";
                    lineStream >> seg;
                    if (seg == "")
                        argv[i++] = NULL;
                    else
                    {
                        argv[i] = new char[seg.length()];
                        std::strcpy(argv[i++], seg.c_str());
                    }
                }
                argv[--i] = NULL;
                std::string command = argv[0];
                execve(command.c_str(), argv, envp);
                std::string tries;
                tries = "/bin/" + command;
                execve(tries.c_str(), argv, envp);
                tries = "/usr/bin/" + command;
                execve(tries.c_str(), argv, envp);
                out << "Error while executing `" << lineCache << "`";
                exit(1);
            }
            // parent
            if (wait)
                ::wait4(pid, NULL, 0, NULL);
            else
                out << "[Background Task Running..., pid:" << pid << "]\n";
            return;
        }

        void printHello()
        {
            out <<
            "Tsh v0.1.2 - A Simple shell made by Tamce.\n"
            "Command Search directory: ., /bin, /usr/bin\n"
            "\n"
            "License: GPLv2\n"
            "Link: https://github.com/tamce/tsh\n";
        }
};

int main(int argc, char **argv, char * const*envp)
{
    TShell tsh(std::cin, std::cout, envp);
    tsh.printHello();
	while (tsh.running())
	{
        tsh.printPS();
        tsh.readLine();
        tsh.exec();
	}
	return 0;
}
