#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void runcommand(char** command, char** path, int size_path) {
    char check[1024];
    for (int i = 0; i < size_path; i++) {
        snprintf(check, 1024, "%s/%s", path[i], command[0]);
        if (access(check, X_OK) == 0) {
            int pid = fork();
            if (pid == 0) {
                char* cla[255];
                memcpy(cla, command, 256);
                execv(check, cla);
            } else {
                wait(NULL);
            }
        } else {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

void mainloop(FILE* file) {
    char** path = malloc(sizeof(char*) * 256);
    path[0] = "/bin";
    int size_path = 1;
    char* input;
    size_t size = 0;
    printf("smash>");
    //while not EOF
    while (getline(&input, &size, file) != -1) {
        //stores the CLA input
        char** out = malloc(sizeof(char*) * 256);
        int count = -1;
        //seperate out each CLA
        while ((out[count + 1] = strtok_r(input, " ", &input)) != NULL) {
            count++;
        }
        if (strcmp(out[0], "\n") != 0) {
            //remove the \n from the last item
            out[count][strcspn(out[count], "\n")] = '\0';
            //exit command
            if (strcmp(out[0], "exit") == 0) {
                if (count == 0) {
                    free(out);
                    fclose(file);
                    exit(0);
                } else {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            } else if (strcmp(out[0], "cd") == 0) {  //cd command
                if (count == 1) {
                    if (chdir(out[1]) != 0) {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                } else {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            } else if (strcmp(out[0], "path") == 0) {  //path command
                if (count >= 1) {
                    if (count == 2) {
                        if (strcmp(out[1], "add") == 0) {  //add
                            path[size_path] = out[2];
                            size_path++;
                        } else if (strcmp(out[1], "remove") == 0) {  //remove
                            for (int i = 0; i < size_path; i++) {
                                if (strcmp(path[i], out[2]) == 0) {
                                    for (int x = i; x < size_path; x++) {
                                        path[x] = path[x + 1];
                                    }
                                    path[size_path] = '\0';
                                    size_path--;
                                }
                            }
                        }
                    } else if (count == 1) {
                        if (strcmp(out[1], "clear") == 0) {  //clear
                            free(path);
                            path = malloc(sizeof(char*) * 256);
                        }
                    } else {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                } else {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            } else {  //other command
                char** command = malloc(sizeof(char*) * 256);
                for (int i = 0; i <= count; i++) {
                    if (strcmp(out[i], "&") != 0 && strcmp(out[i], ">") != 0 && strcmp(out[i], ";") != 0 && strcmp(out[i], "") != 0 && out[i][0] != '\0') {
                        command[i] = out[i];
                    }
                }
                runcommand(command, path, size_path);
            }
        }
        printf("smash>");
    }
    fclose(file);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        mainloop(stdin);
    } else if (argc == 2) {
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        mainloop(file);
    } else {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}
