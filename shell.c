#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DELIMETERS " \n\r"

char history[5000][5000];
int append = 0;
long histVal;
int histFlag = 0;
int redirectOut = 0;
int redirectIn = 0;

// Hold integer for number of parameters in command
typedef struct NumArgs {
    int num;
} NumArgs;

// Function reads a line from the keyboard
char * readLine(char buffer[]) {

    char * line;
    fgets(buffer,200,stdin);
    line = malloc(sizeof(char)*(strlen(buffer)));
    strcpy(line,buffer);
    return line;
}

// Function Parses lines parameters into an array
char ** parseLine(char * line, NumArgs * totalArg) {

    char ** args;
    char * arg;
    int bufSize = 100;
    int pos = 0;

    if (strcmp(line, "\n") == 0) {
        args = NULL;
        return args;
    }

    // Looks for delimters " ", "\n", "\r"
    args = malloc(bufSize * sizeof(char *));
    arg = strtok(line, DELIMETERS);
    totalArg->num++;

    while(arg != NULL) {
        args[pos] = arg;
        pos++;

        args = realloc(args, bufSize * sizeof(char *) + 1);
        arg = strtok(NULL, DELIMETERS);
        totalArg->num++;
    }
    args[pos] = NULL;
    return args;
}

void addToHistory(char buffer[]) {
    strcpy(history[append], buffer);
    append++;
}

void printHistory() {
    if (histFlag == 1) {
        if (histVal > append) {
            for (int i = 0; i < append; i++)
                printf("%d   %s", i + 1, history[i]);
            return;
        }
        for (int i = append - histVal; i < append; i++)
            printf("%d   %s", i + 1, history[i]);
    }
    else {
        for (int i = 0; i < append; i++)
            printf("%d   %s", i + 1, history[i]);
    }
    histFlag = 0;
}

void checkFlags(int numArgs, char ** args) {

    for (int i = 0; i < numArgs - 1; i++) {
        if (strcmp(args[i],">") == 0)
            redirectOut = i;
        if (strcmp(args[i],"<") == 0)
            redirectIn = i;
    }
    // printf("in: %d\n", redirectIn);
    // printf("out: %d\n", redirectOut);

}

// Forks cild process to execute the command entered
// Error message will be returned if command is invalid
void runCommand(char * line, char ** args, NumArgs * totalArg) {

    pid_t pid;
    int noCmd = 0;
    FILE * file = NULL;

    if (strcmp(args[0], "history") == 0) {
        histVal = strtol(args[1], args, histVal);
        if (histVal == 0) {
            printf("-bash: history: %s: numeric argument required\n", args[1]);
            return;
        }
        histFlag = 1;
        printHistory();
        return;
    }

    pid = fork();
    if (pid < 0) {
        printf("Error forking\n");
        exit(0);
    }
    else if (pid == 0) { //child

        if (strcmp(args[0], "exit") == 0)
            exit(-1);
        
        // if (redirectOut > 0)
        //     file = freopen(args[redirectOut + 1], "w+", stdout);
        noCmd = execvp(args[0], args);
        fclose(file);
        
        if (noCmd == -1)
            printf("-bash: %s: command not found\n", line);
        //fclose(file);
        exit(-1);
    }
    else {
        wait(NULL);
    }
}//end runProgram()

// Contains command loop calling functions for reading / parsing / executing line
// Stores commands entered in history data structure
// If user enters a newline character "\n" the loop repeats itself
// Executes until the user enters "exit"
// Calls functions to free all malloc'd memory
// Prints logout messages
int main (int argc, char * argv[]) {

    char * line;
    char ** args;
    do {

        printf("> ");
        char buffer[200];
        histVal = 0;
        NumArgs * totalArg;
        totalArg = malloc(sizeof(NumArgs));
        redirectIn = 0;
        redirectOut = 0;
        totalArg->num = 0;

        line = readLine(buffer);
        addToHistory(buffer);

        if (strcmp(line, "history\n") == 0) {
            printHistory();
            continue;
        }

        args = parseLine(line, totalArg);
        if (args == NULL)
            continue;

        checkFlags(totalArg->num, args);
        runCommand(line, args, totalArg);

        if (strcmp(line, "exit") == 0)
            break;

        free(line);
        free(args);
        free(totalArg);

        } while (strcmp(line, "exit") != 0);

    printf("logout\n");
    printf("\n");
    printf("[Process completed]\n");

    return 0;
}




