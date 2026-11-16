#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

// Wrapper executable that allows to call mpi with the same executable of
// regular program
int main(int argc, char* argv[]) {
    bool        distribute{false};
    std::string token{"--distribute"};
    for(int i{1}; i < argc; ++i) {
        if(token == argv[i]) {
            distribute = true;
            break;
        }
    }
    std::string command;
    if(distribute) {
        command = "mpiexec --npernode 1 --hostfile hfile  --mca "
                  "btl_tcp_if_exclude eno4,docker0  __MPI_RunSelector__ "
                  "--window_size ";
    } else {
        command = "__MPI_RunSelector__ --window_size ";
    }

    struct winsize w {};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    command += std::to_string(w.ws_col);

    for(int i{1}; i < argc; ++i) {
        command += " ";
        command += argv[i];
    }
    std::cout << command << std::endl;
    system(command.c_str());
}
