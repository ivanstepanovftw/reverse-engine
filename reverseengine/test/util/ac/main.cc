#include <iostream>
#include <sstream>
#include <zconf.h>
#include <sys/ptrace.h>

bool is_ld_preload() {
    char *ldp = getenv("LD_PRELOAD");
    std::cout << "ldp (" << (void *) ldp << ") :" << ldp << std::endl;
    return false;
}

bool is_valgrind() {
    return false;
}

bool is_ptrace() {
    return ptrace(PTRACE_TRACEME, 0, 1, 0) == -1;
}

bool is_perf() {
    return false;
}


#include <unistd.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <cstring>

#if !defined(PTRACE_ATTACH) && defined(PT_ATTACH)
#  define PTRACE_ATTACH PT_ATTACH
#endif
#if !defined(PTRACE_DETACH) && defined(PT_DETACH)
#  define PTRACE_DETACH PT_DETACH
#endif

#ifdef __linux__
#  define _PTRACE(_x, _y) ptrace(_x, _y, NULL, NULL)
#else
#  define _PTRACE(_x, _y) ptrace(_x, _y, NULL, 0)
#endif

/** Determine if we're running under a debugger by attempting to attach using pattach
 *
 * @return 0 if we're not, 1 if we are, -1 if we can't tell.
 */
static int debugger_attached(void)
{
    int pid;

    int from_child[2] = {-1, -1};

    if (pipe(from_child) < 0) {
        fprintf(stderr, "Debugger check failed: Error opening internal pipe: %s", std::strerror(errno));
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Debugger check failed: Error forking: %s", std::strerror(errno));
        return -1;
    }

    /* Child */
    if (pid == 0) {
        uint8_t ret = 0;
        int ppid = getppid();

        /* Close parent's side */
        close(from_child[0]);

        if (_PTRACE(PTRACE_ATTACH, ppid) == 0) {
            /* Wait for the parent to stop */
            waitpid(ppid, NULL, 0);

            /* Tell the parent what happened */
            write(from_child[1], &ret, sizeof(ret));

            /* Detach */
            _PTRACE(PTRACE_DETACH, ppid);
            exit(0);
        }

        ret = 1;
        /* Tell the parent what happened */
        write(from_child[1], &ret, sizeof(ret));

        exit(0);
        /* Parent */
    } else {
        uint8_t ret = -1;

        /*
         *  The child writes a 1 if pattach failed else 0.
         *
         *  This read may be interrupted by pattach,
         *  which is why we need the loop.
         */
        while ((read(from_child[0], &ret, sizeof(ret)) < 0) && (errno == EINTR));

        /* Ret not updated */
        if (ret < 0) {
            fprintf(stderr, "Debugger check failed: Error getting status from child: %s", std::strerror(errno));
        }

        /* Close the pipes here, to avoid races with pattach (if we did it above) */
        close(from_child[1]);
        close(from_child[0]);

        /* Collect the status of the child */
        waitpid(pid, NULL, 0);

        return ret;
    }
}


int main(int argc, char *argv[]) {
    using namespace std;
    // http://www.ouah.org/linux-anti-debugging.txt
    // http://www.ouah.org/linux-anti-debugging.txt
    // http://www.ouah.org/linux-anti-debugging.txt
    // http://www.ouah.org/linux-anti-debugging.txt
    cout << "Hello: " << (char *) (0) << endl;
    cout << "world..." << endl;

    cout << "pid: " << getpid() << endl;
    cout << "ppid: " << getppid() << endl;
    cout << "argv[0]: " << argv[0] << endl;

    cout << "is_ld_preload: " << (is_ld_preload()) << endl;
    // cout<<"is_valgrind: "<<(is_valgrind())<<endl;
    // cout<<"is_ptrace: "<<(is_ptrace())<<endl;
    // cout<<"is_perf: "<<(is_perf())<<endl;
    cout << "No Error" << endl;
    return 0;
}
