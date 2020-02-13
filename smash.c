#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int size_path = 1;

// TODO: Make all error messages run correctly and when they are supposed to
// TODO: empty commands

void path_command(char** out, int count, char** path) {
    if (count == 3) {
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
    } else if (count == 2) {
        if (strcmp(out[1], "clear") == 0) {  //clear
            size_path = 0;
        }
    }
}
void cd(char** out, int count) {
    if (chdir(out[1]) != 0) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

void runcommand(char** command, int size_command, char** path, FILE* redirection) {
    char check[1024];
    if (redirection != NULL) {
        int newfd = fileno(redirection);
        dup2(newfd, 1);
        dup2(newfd, 2);
    }
    for (int i = 0; i < size_path; i++) {
        //build each path with executable
        snprintf(check, 1024, "%s/%s", path[i], command[0]);
        //check if executable can run
        if (access(check, X_OK) == 0) {
            //run the command
            execv(check, command);
        }
    }
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void parsecommand(char** command, int size_command, char** path) {
    char*and = "&";
    char* semicolon = ";";
    char** allcommands[256];
    int lengthcommand[256];
    for (int o = 0; o < 256; o++) {
        lengthcommand[o] = 0;
    }
    int numcommands = 0;
    int doulberedirect = -1;
    char** cla = malloc(sizeof(char*) * 256);
    int i = 0;
    int size = 0;
    FILE** redirection = malloc(sizeof(FILE*) * 256);
    //run through each CLA and handle special syntax
    while (i < size_command) {
        //create a file for redirection and place in array
        if (strcmp(command[i], ">") == 0) {
            if (doulberedirect != 1) {
                i++;
                redirection[numcommands] = fopen(command[i], "w");
                i++;
            }
            if (i < size_command && strcmp(command[i], ">") == 0) {
                doulberedirect = 1;
            }
        } else if (strcmp(command[i], "&") == 0) {
            allcommands[numcommands] = cla;
            lengthcommand[numcommands] = size;
            numcommands++;
            allcommands[numcommands] = &and;
            lengthcommand[numcommands] = 1;
            numcommands++;
            size = 0;
            cla = malloc(sizeof(char*) * 256);
            i++;
        } else if (strcmp(command[i], ";") == 0) {
            allcommands[numcommands] = cla;
            lengthcommand[numcommands] = size;
            numcommands++;
            allcommands[numcommands] = &semicolon;
            lengthcommand[numcommands] = 1;
            numcommands++;
            size = 0;
            i++;
            cla = malloc(sizeof(char*) * 256);
        } else {
            cla[size] = command[i];
            size++;
            i++;
        }
    }
    allcommands[numcommands] = cla;
    lengthcommand[numcommands] = size;
    numcommands++;
    char check[1024];
    int* isgood = calloc(numcommands, sizeof(int));
    //check to see if all commands are valid
    for (int x = 0; x < numcommands; x++) {
        if (strcmp(allcommands[x][0], "cd") == 0) {  //check if built in commands
            if (lengthcommand[x] == 2) {
                isgood[x] = 1;
            }
        } else if (strcmp(allcommands[x][0], "path") == 0) {
            if (lengthcommand[x] == 2) {
                if (strcmp(allcommands[x][1], "clear") == 0) {
                    isgood[x] = 1;
                }
            } else if (lengthcommand[x] == 3) {
                if (strcmp(allcommands[x][1], "add") == 0 || strcmp(allcommands[x][1], "remove") == 0) {
                    isgood[x] = 1;
                }
            }
        } else if (strcmp(allcommands[x][0], "exit") == 0) {
            if (lengthcommand[x] == 1) {
                isgood[x] = 1;
            }
        } else if (strcmp(allcommands[x][0], "&") == 0 || strcmp(allcommands[x][0], ";") == 0) {  //allow syntax
            isgood[x] = 1;
        } else {
            for (int j = 0; j < size_path; j++) {  //check external commands
                snprintf(check, 1024, "%s/%s", path[j], allcommands[x][0]);
                if (access(check, X_OK) == 0) {
                    isgood[x] = 1;
                }
            }
        }
    }
    //if one of the commands is invalid do not run
    int checkgood = 1;
    for (int x = 0; x < numcommands; x++) {
        if (isgood[x] != 1) {
            checkgood = -1;
        }
    }
    if (doulberedirect == 1) {
        checkgood = -1;
    }
    //don't run commands when invalid
    if (checkgood != 1) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        //run all commands
        int children = 0;
        for (int x = 0; x < numcommands; x++) {
            if (strcmp(allcommands[x][0], ";") == 0) {
                for (int j = 0; j < children; j++) {
                    wait(NULL);
                    // wait for all commands currently running to finish
                }
                children = 0;
            } else if (strcmp(allcommands[x][0], "&") == 0) {
                // skip
            } else {
                if (strcmp(allcommands[x][0], "cd") == 0) {
                    cd(allcommands[x], lengthcommand[x]);
                } else if (strcmp(allcommands[x][0], "path") == 0) {
                    path_command(allcommands[x], lengthcommand[x], path);
                } else if (strcmp(allcommands[x][0], "exit") == 0) {
                    free(cla);
                    for (int j = 0; j < 256; j++) {
                        if (redirection[j] != NULL) {
                            fclose(redirection[j]);
                        }
                    }
                    free(redirection);
                    exit(0);
                } else {
                    int pid = fork();
                    if (pid == 0) {
                        runcommand(allcommands[x], lengthcommand[x], path, redirection[x]);
                    } else {
                        children++;
                    }
                }
            }
        }
        for (int x = 0; x < children; x++) {
            wait(NULL);
        }
    }
}

void mainloop(FILE* file) {
    char** path = malloc(sizeof(char*) * 256);
    path[0] = "/bin";

    char* input;
    size_t size = 0;
    if (file == stdin) {  //don't write prompt to file
        printf("smash> ");
        fflush(stdout);
    }
    //while not EOF
    while (getline(&input, &size, file) != -1) {
        //stores the CLA input
        char** out = malloc(sizeof(char*) * 256);
        int count = -1;
        //seperate out each CLA
        while ((out[count + 1] = strtok_r(input, " ", &input)) != NULL) {
            char* tabs;
            //remove tabs
            while ((tabs = strstr(out[count + 1], "\t")) != NULL) {
                int index = tabs - out[count + 1];
                memmove(&out[count + 1][index], &out[count + 1][index] + 1, strlen(out[count + 1]) - index);
            }
            count++;
        }
        //if just an endline rerun loop
        if (strcmp(out[0], "\n") != 0) {
            //remove the \n from the last item
            out[count][strcspn(out[count], "\n")] = '\0';
            //exit command
            if (strcmp(out[0], "exit") == 0) {
                if (count == 0) {
                    exit(0);
                } else {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            } else {  //other command
                char** command = malloc(sizeof(char*) * 256);
                int size_command = 0;
                char* syntax[3] = {";", "&", ">"};
                int length[3] = {-1, -1, -1};
                //create an array with each CLA its own element
                for (int i = 0; i <= count; i++) {
                    char** found = malloc(sizeof(char*) * 3);
                    if (strcmp(out[i], "") != 0 && out[i][0] != '\0') {
                        //seperate out if no space
                        if ((found[0] = strstr(out[i], ";")) != NULL || (found[1] = strstr(out[i], "&")) != NULL || (found[2] = strstr(out[i], ">")) != NULL) {
                            for (int k = 0; k < 3; k++) {
                                length[k] = -1;
                            }
                            int which = -1;
                            if (found[0] != NULL) {
                                which = 0;
                                length[0] = strlen(found[0]);
                                found[1] = strstr(out[i], "&");
                                found[2] = strstr(out[i], ">");
                            }
                            if (found[1] != NULL) {
                                which = 1;
                                length[1] = strlen(found[1]);
                                found[2] = strstr(out[i], ">");
                            }
                            if (found[2] != NULL) {
                                which = 2;
                                length[2] = strlen(found[2]);
                            }
                            if (length[0] > length[1] && length[0] > length[2]) {
                                which = 0;
                            } else if (length[1] > length[0] && length[1] > length[2]) {
                                which = 1;
                            } else if (length[2] > length[1] && length[2] > length[0]) {
                                which = 2;
                            }
                            if (strlen(out[i]) - strlen(found[which]) != 0) {
                                command[size_command] = malloc(1 + strlen(out[i]) - strlen(found[which]));
                                strncpy(command[size_command], out[i], strlen(out[i]) - strlen(found[which]));
                                command[size_command][strlen(out[i]) - strlen(found[which])] = '\0';
                                size_command++;
                            }
                            command[size_command] = syntax[which];
                            size_command++;
                            found[which]++;
                            char** newfound = malloc(sizeof(char*) * 3);
                            int newwhich = -1;
                            //while there are still more syntax and commands with no space seperating
                            while ((newfound[0] = strstr(found[which], ";")) != NULL || (newfound[1] = strstr(found[which], "&")) != NULL || (newfound[2] = strstr(found[which], ">")) != NULL) {
                                for (int k = 0; k < 3; k++) {
                                    length[k] = -1;
                                }
                                if (newfound[0] != NULL) {
                                    newwhich = 0;
                                    length[0] = strlen(newfound[0]);
                                    newfound[1] = strstr(found[which], "&");
                                    newfound[2] = strstr(found[which], ">");
                                }
                                if (newfound[1] != NULL) {
                                    newwhich = 1;
                                    length[1] = strlen(newfound[1]);
                                    newfound[2] = strstr(found[which], ">");
                                }
                                if (newfound[2] != NULL) {
                                    newwhich = 2;
                                    length[2] = strlen(newfound[2]);
                                }
                                if (length[0] > length[1] && length[0] > length[2]) {
                                    newwhich = 0;
                                } else if (length[1] > length[0] && length[1] > length[2]) {
                                    newwhich = 1;
                                } else if (length[2] > length[1] && length[2] > length[0]) {
                                    newwhich = 2;
                                }
                                if (strlen(found[which]) - strlen(newfound[newwhich]) != 0) {
                                    command[size_command] = malloc(1 + strlen(found[which]) - strlen(newfound[newwhich]));
                                    strncpy(command[size_command], found[which], strlen(found[which]) - strlen(newfound[newwhich]));
                                    command[size_command][strlen(found[which]) - strlen(newfound[newwhich])] = '\0';
                                    size_command++;
                                }
                                command[size_command] = syntax[newwhich];
                                size_command++;
                                found[newwhich] = newfound[newwhich] + 1;
                                which = newwhich;
                            }
                            free(newfound);
                            if (strcmp(found[which], "") != 0) {
                                command[size_command] = found[which];
                                size_command++;
                            }
                        } else {
                            command[size_command] = out[i];
                            size_command++;
                        }
                    }
                    free(found);
                }
                parsecommand(command, size_command, path);
                free(command);
            }
        }
        if (file == stdin) {
            printf("smash> ");
            fflush(stdout);
        }
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
