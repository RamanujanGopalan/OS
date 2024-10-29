#include <signal.h>

int dummy_main(int argc, char **argv);
int main(int argc, char **argv) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    int ret = dummy_main(argc, argv);
    return ret;
}
#define main dummy_main
