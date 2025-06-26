#include <unistd.h>
#include <sys/wait.h>

int main() {


    // gera teste.txt
    int nnumbers;
    int ntasks;
    int nthreads;
    char str[20][3];
    sprintf(str[0], "%d", nnumbers);
    sprintf(str[1], "%d", ntasks);
    sprintf(str[2], "%d", nthreads);
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"./sort", "teste.txt", str[0], str[1], str[2], NULL};
        execvp(args[0], args);
    } else {
        wait(NULL);  // espera o filho terminar
    }

    // compara resultado

    return 0;
}
