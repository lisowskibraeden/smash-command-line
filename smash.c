#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void parsecommand(char** command, int size_command, char** path, int size_path) {
    printf("ehlo\n");
}

void runcommand(char** command, int size_command, char** path, int size_path) {
    char check[1024];
    for (int i = 0; i < size_path; i++) {
        //build each path with executable
        snprintf(check, 1024, "%s/%s", path[i], command[0]);
        //check if executable can run
        if (access(check, X_OK) == 0) {
            //run the command
            int pid = fork();
            if (pid == 0) {
                //child runs
                char* cla[255];
                memcpy(cla, command, 256);
                execv(check, cla);
            } else {
                //parent waits until done
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
        //if just an endline rerun loop
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
                int size_command = 0;
                for (int i = 0; i <= count; i++) {
                    if (strcmp(out[i], "") != 0 && out[i][0] != '\0') {
                        char* found;
                        char* lastfound;
                        if ((found = strstr(out[i], "&")) != NULL) {
                            do {
                                command[size_command] = malloc(1 + found - out[i]);
                                strncpy(command[size_command], out[i], found - out[i]);
                                command[size_command][found - out[i]] = '\0';
                                if(strcmp(command[size_command], "") == 0){
                                    free(command[size_command]);
                                    size_command--;
                                }
                                size_command++;
                                command[size_command] = malloc(1);
                                strncpy(command[size_command], found, 1);
                                size_command++;
                                found++;
                                lastfound = found;
                            } while ((found = strstr(found, "&")) != NULL);
                            if (strcmp(lastfound, "") != 0) {
                                command[size_command] = lastfound;
                                size_command++;
                            }
                        } else {
                            command[size_command] = out[i];
                            size_command++;
                        }
                    }
                }
                printf("print\n");
                for (int i = 0; i < size_command; i++) {
                    printf("index: %d string: %s\n", i, command[i]);
                }
                parsecommand(command, size_command, path, size_path);
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
