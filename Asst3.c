#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define BACKLOG 1

/**
 *  This function checks for any error in the message received from the client 
*/
int errorChecker(char *buf, char *expectedStr, int crrtLength, int readNumber)
{
    int i = 0;
    int lastIndex = 4;
    // The for loop is used to check if the char is a digit at the place where content lenght is specified
    for (i = 4; i < strlen(buf); i++)
    {
        if (i == 4 && !(isdigit(buf[i])))
        {
            return 1;
        }
        else if (isdigit(buf[i]))
        {
            lastIndex = i;
        }
        else
        {
            break;
        }
    }
    // The if conditionals below check is the messgae starts with REG or ERR
    if (buf[0] == 'R')
    {
        if (buf[1] == 'E')
        {
            if (buf[2] != 'G')
            {
                return 2;
            }
        }
        else
        {
            return 2;
        }
    }
    else if (buf[0] == 'E')
    {
        if (buf[1] == 'R')
        {
            if (buf[2] != 'R')
            {
                return 2;
            }
            else
            {
                return 10;
            }
        }
        else
        {
            return 2;
        }
    }
    else
    {
        return 2;
    }
    //This if checks if second pipe is at right place
    if (buf[lastIndex + 1] != '|')
    {
        return 2;
    }

    // Store the message content in a variable for later use
    char *tempString = (char *)malloc(sizeof(char *) * (lastIndex - 3));
    for (i = 0; i <= (lastIndex - 4); i++)
    {
        tempString[i] = buf[i + 4];
    }
    tempString[i] = '\0';
    int length = atoi(tempString);
    // Checks if last pipe exists and is at right place
    if (buf[lastIndex + 1 + length + 1] != '|')
    {
        return 2;
    }
    if (length != crrtLength)
    {
        return 1;
    }
    free(tempString);
    tempString = (char *)malloc(sizeof(char *) * (length + 4));
    for (i = 0; i < (length); i++)
    {
        tempString[i] = buf[lastIndex + 2 + i];
    }
    tempString[i] = '\0';
    // Checks if expression of annoyance ends with a punctuation mark
    if (readNumber == 3)
    {
        if (tempString[strlen(tempString) - 1] != '!')
        {
            if (tempString[strlen(tempString) - 1] != '.')
            {
                if (tempString[strlen(tempString) - 1] != '?')
                {
                    return 0;
                }
            }
        }
    }
    if (strcmp(tempString, expectedStr) != 0)
    {
        return 0;
    }
    free(tempString);
    return -1;
}

// the argument we will pass to the connection-handler
struct connection
{
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

char *errCodes[18];
char *errMessage[18];
int k = 0;
int totallines = 0;
char *jokesList;

int server(char *port);
void *echo(void *arg);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc == 3)
    {
        int fd = open(argv[2], O_RDONLY);
        if (fd < 0)
        {
            printf("Unable to open the file. Will use default joke.\n");
            jokesList = NULL;
        }
        else
        {
            // copy the content of joke files into a string
            int bytes, pos;
            int filesize = 0;
            struct stat st;
            stat(argv[2], &st);
            filesize = (int)st.st_size;

            char *buffer = (char *)malloc(sizeof(char) * filesize);
            char *buffercopy = (char *)malloc(sizeof(char) * filesize);
            jokesList = (char *)malloc(sizeof(char) * filesize);
            bytes = read(fd, buffer, filesize);
            if (bytes < 0)
                printf("error\n");

            buffercopy = strcpy(buffercopy, buffer);

            char *token = strtok(buffercopy, "\n\f\r");
            int notAdded = 1;
            strcpy(jokesList, token);
            notAdded = 0;
            int totalwords = 0;
            while (token != NULL)
            {
                totallines++;
                if (notAdded)
                    strcat(jokesList, token);
                jokesList[strlen(jokesList)] = '\n';
                token = strtok(NULL, "\n\r\f");
                notAdded = 1;
            }
            jokesList[strlen(jokesList)] = '\0';
        }
        close(fd);
    }
    else
    {
        jokesList = NULL;
    }
    
    // load error codes
    for (k = 0; k < 18; k++)
    {
        errCodes[k] = (char *)malloc(sizeof(char *) * 10);
    }
    strcpy(errCodes[0], "M0CT");
    strcpy(errCodes[1], "M0LN");
    strcpy(errCodes[2], "M0FT");
    strcpy(errCodes[3], "M1CT");
    strcpy(errCodes[4], "M1LN");
    strcpy(errCodes[5], "M1FT");
    strcpy(errCodes[6], "M2CT");
    strcpy(errCodes[7], "M2LN");
    strcpy(errCodes[8], "M2FT");
    strcpy(errCodes[9], "M3CT");
    strcpy(errCodes[10], "M3LN");
    strcpy(errCodes[11], "M3FT");
    strcpy(errCodes[12], "M4CT");
    strcpy(errCodes[13], "M4LN");
    strcpy(errCodes[14], "M4FT");
    strcpy(errCodes[15], "M5CT");
    strcpy(errCodes[16], "M5LN");
    strcpy(errCodes[17], "M5FT");
    // load error messages
    for (k = 0; k < 18; k++)
    {
        errMessage[k] = (char *)malloc(sizeof(char *) * 1000);
    }
    strcpy(errMessage[0], "message 0 content was not correct (i.e. should be \"Knock, knock.\")");
    strcpy(errMessage[1], "message 0 length value was incorrect (i.e. should be 13 characters long)");
    strcpy(errMessage[2], "message 0 format was broken (did not include a message type, missing or too many \'|\')");
    strcpy(errMessage[3], "message 1 content was not correct (i.e. should be \"Who\'s there?\")");
    strcpy(errMessage[4], "message 1 length value was incorrect (i.e. should be 12 characters long)");
    strcpy(errMessage[5], "message 1 format was broken (did not include a message type, missing or too many \'|\')");
    strcpy(errMessage[6], "message 2 content was not correct (i.e. missing punctuation)");
    strcpy(errMessage[7], "message 2 length value was incorrect (i.e. should be the length of the message)");
    strcpy(errMessage[8], "message 2 format was broken (did not include a message type, missing or too many \'|\')");
    strcpy(errMessage[9], "message 3 content was not correct(i.e. should contain message 2 with \", who?\" tacked on)");
    strcpy(errMessage[10], "message 3 length value was incorrect (i.e. should be M2 length plus six)");
    strcpy(errMessage[11], "message 3 format was broken (did not include a message type, missing or too many \'|\')");
    strcpy(errMessage[12], "message 4 content was not correct (i.e. missing punctuation)");
    strcpy(errMessage[13], "message 4 length value was incorrect (i.e. should be the length of the message)");
    strcpy(errMessage[14], "message 4 format was broken (did not include a message type, missing or too many \'|\')");
    strcpy(errMessage[15], "message 5 content was not correct (i.e. missing punctuation)");
    strcpy(errMessage[16], "message 5 length value was incorrect (i.e. should be the length of the message)");
    strcpy(errMessage[17], "message 5 format was broken (did not include a message type, missing or too many \'|\')");

    (void)server(argv[1]);
    for (k = 0; k < 18; k++)
    {
        free(errCodes[k]);
    }
    for (k = 0; k < 18; k++)
    {
        free(errMessage[k]);
    }
    return EXIT_SUCCESS;
}

/**
 *  This function counts the number of pipes in the message received from the client 
*/
int countPipes(char *buf)
{
    int i = 0;
    int pipes = 0;
    for (i = 0; i < strlen(buf); i++)
    {
        if (buf[i] == '|')
            pipes++;
    }
    return pipes;
}

int server(char *port)
{
    struct addrinfo hint, *address_list, *addr;
    struct connection *con;
    int error, sfd;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL, port, &hint, &address_list);
    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (addr = address_list; addr != NULL; addr = addr->ai_next)
    {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        // if we couldn't create the socket, try the next method
        if (sfd == -1)
        {
            continue;
        }

        if ((bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) &&
            (listen(sfd, BACKLOG) == 0))
        {
            break;
        }

        // unable to set it up, so try the next method
        close(sfd);
    }

    if (addr == NULL)
    {
        // we reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(address_list);

    // at this point sfd is bound and listening
    printf("Waiting for connection\n");
    for (;;)
    {
        con = malloc(sizeof(struct connection));
        con->addr_len = sizeof(struct sockaddr_storage);

        // wait for an incoming connection
        con->fd = accept(sfd, (struct sockaddr *)&con->addr, &con->addr_len);

        if (con->fd == -1)
        {
            perror("accept");
            continue;
        }

        echo(con);
    }

    return 0;
}

/**
 *  This function sends and receives messages to and from the client
*/
void *echo(void *arg)
{
    char host[100], port[10], buf[501];
    struct connection *c = (struct connection *)arg;
    int error, nread;

    // find out the name and port of the remote host
    error = getnameinfo((struct sockaddr *)&c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);

    if (error != 0)
    {
        fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        close(c->fd);
        return NULL;
    }

    // write the message to initiate a conversation
    write(c->fd, "REG|13|Knock, knock.|", strlen("REG|13|Knock, knock.|"));
    printf("%s\n", "Knock, knock.");
    int pipeCount = 0;
    int loopCount = 0;
    int contentLength = 0;
    char *lengthString = (char *) malloc(sizeof(char *) * 10);
    int lengthStringTracker = 0;
    char tm[500];
    int tmTr = 0;
    // Keep reading from the client until cannot read anymore or three pipes are read. whichever occurs first
    while (1)
    {
        bzero(buf, 500);
        if (pipeCount < 2)
        {
            nread = read(c->fd, buf, 1);
            if(pipeCount == 1 && buf[0] != '|') {
                if(!isdigit(buf[0])) {
                    write(c->fd, "ERR|M1LN|", strlen("ERR|M1LN|"));
                    printf("%s\n", errMessage[4]);
                    close(c->fd);
                    free(c);
                    return NULL;
                }
		if(lengthStringTracker == 0)
				lengthString = (char *) malloc(sizeof(char *) * 10);
                lengthString[lengthStringTracker] = buf[0];
                lengthStringTracker++;
            }
            tm[tmTr] = buf[0];
            tmTr++;
        }
        else
        {
		    lengthString[lengthStringTracker] = '\0';
            contentLength = atoi(lengthString);
            if(contentLength != 12) {
                write(c->fd, "ERR|M1LN|", strlen("ERR|M1LN|"));
                printf("%s\n", errMessage[4]);
                close(c->fd);
                free(c);
                return NULL;
            }
            nread = read(c->fd, buf, contentLength + 1);
            int j = 0;
            for (j = 0; j < (contentLength + 1); j++)
            {
                tm[tmTr] = buf[j];
                tmTr++;
            }
        }
        if (nread > 0)
        {
            buf[nread] = '\0';
            pipeCount += countPipes(buf);
            if (pipeCount == 3)
            {
                break;
            }
        }
        else {
            printf("%s\n", errMessage[5]);
            break;
        }
    }
    tm[tmTr] = '\0';
    int errValue = errorChecker(tm, "Who's there?", 12, 1);

    if (errValue == 0)
    {
        write(c->fd, "ERR|M1CT|", strlen("ERR|M1CT|"));
        printf("%s\n", errMessage[3]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 1)
    {
        write(c->fd, "ERR|M1LN|", strlen("ERR|M1LN|"));
        printf("%s\n", errMessage[4]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 2)
    {
        write(c->fd, "ERR|M1FT|", strlen("ERR|M1FT|"));
        printf("%s\n", errMessage[5]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 10)
    {
        printf("%s\n", tm);
        close(c->fd);
        free(c);
        return NULL;
    }

    printf("%s\n", "Who's there?");
    free(lengthString);
    lengthStringTracker = 0;
    if (totallines == 0)
        totallines = 1;
    // If a joke file was passed, select a joke randomly
    int setupLineNumber = rand() % totallines;
    if (setupLineNumber % 2 == 0)
    {
        setupLineNumber = setupLineNumber + 1;
    }
    int punchLineNumber = setupLineNumber + 1;

    char *setupLine;
    char *punchLine;
    int tracker = 1;
    if (jokesList == NULL)
    {
        // default joke if no file was passed
    	setupLine = "Broken Pencil.\0";
        punchLine = "Never mind it's pointless.\0";
    }
    else
    {
        char *jokesCopy = (char *)malloc(sizeof(char *) * ((int)strlen(jokesList) + 1));
        strcpy(jokesCopy, jokesList);
        char *token = strtok(jokesCopy, "\n\f\r");

        while (token != NULL)
        {
            if (tracker == setupLineNumber)
            {
                setupLine = (char *)malloc(sizeof(char *) * ((int)strlen(token) + 5));
                strcpy(setupLine, token);
                setupLine[strlen(token)] = '\0';
            }
            if (tracker == punchLineNumber)
            {
                punchLine = (char *)malloc(sizeof(char *) * ((int)strlen(token) + 5));
                strcpy(punchLine, token);
                punchLine[strlen(token)] = '\0';
                break;
            }
            tracker++;
            token = strtok(NULL, "\n\r\f");
        }
    }
    char *jokePointer = (char *)malloc(sizeof(char *) * 500);
    strcpy(jokePointer, "REG|");
    char *tempLength = (char *)malloc(sizeof(char *) * 500);
    sprintf(tempLength, "%d", (int)strlen(setupLine));
    strcat(jokePointer, tempLength);
    strcat(jokePointer, "|");
    strcat(jokePointer, setupLine);
    strcat(jokePointer, "|");
    jokePointer[strlen(jokePointer)] = '\0';
    // Send the setup line to client
    write(c->fd, jokePointer, strlen(jokePointer));
    printf("%s\n", setupLine);

    tmTr = 0;
    bzero(tm, 100);
    pipeCount = 0;
    lengthString = (char *) malloc(sizeof(char *) * 10);
    lengthStringTracker = 0;

    while (1)
    {
        bzero(buf, 100);
        if (pipeCount < 2)
        {
            nread = read(c->fd, buf, 1);
            if(pipeCount == 1 && buf[0] != '|') {
                if(!isdigit(buf[0])) {
                    write(c->fd, "ERR|M3LN|", strlen("ERR|M3LN|"));
                    printf("%s\n", errMessage[10]);
                    close(c->fd);
                    free(c);
                    return NULL;
                }
                lengthString[lengthStringTracker] = buf[0];
                lengthStringTracker++;
            }
            tm[tmTr] = buf[0];
            tmTr++;
        }
        else
        {
            lengthString[lengthStringTracker] = '\0';
            contentLength = atoi(lengthString);
            if(contentLength != ((int) strlen(setupLine) + 5)) {
                write(c->fd, "ERR|M3LN|", strlen("ERR|M3LN|"));
                printf("%s\n", errMessage[10]);
                close(c->fd);
                free(c);
                return NULL;
            }
            nread = read(c->fd, buf, contentLength + 1);
            int j = 0;
            for (j = 0; j < (contentLength + 1); j++)
            {
                tm[tmTr] = buf[j];
                tmTr++;
            }
        }
        if (nread > 0)
        {
            buf[nread] = '\0';
            if(countPipes(buf) == 1 && buf[nread - 1] != '|') {
                write(c->fd, "ERR|M3LN|", strlen("ERR|M3LN|"));
                printf("%s\n", errMessage[10]);
                close(c->fd);
                free(c);
                return NULL;
            }
            pipeCount += countPipes(buf);
            if (pipeCount == 3)
            {
                break;
            }
        }
        else {
            printf("%s\n", errMessage[11]);
            break;
        }
    }
    tm[tmTr] = '\0';
    char *tempErrorString = (char *)malloc(sizeof(char *) * 500);
    strcpy(tempErrorString, setupLine);
    tempErrorString[(int)strlen(tempErrorString) - 1] = '\0';
    strcat(tempErrorString, ", who?");
    tempErrorString[strlen(tempErrorString)] = '\0';
    errValue = errorChecker(tm, tempErrorString, (int)strlen(tempErrorString), 2);

    if (errValue == 0)
    {
        write(c->fd, "ERR|M3CT|", strlen("ERR|M3CT|"));
        printf("%s\n", errMessage[9]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 1)
    {
        write(c->fd, "ERR|M3LN|", strlen("ERR|M3LN|"));
        printf("%s\n", errMessage[10]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 2)
    {
        write(c->fd, "ERR|M3FT|", strlen("ERR|M3FT|"));
        printf("%s\n", errMessage[11]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 10)
    {
        printf("%s\n", tm);
        close(c->fd);
        free(c);
        return NULL;
    }
    printf("%s\n", tempErrorString);
    free(tempErrorString);
    free(tempLength);
    free(jokePointer);
    free(lengthString);
    lengthStringTracker = 0;

    jokePointer = (char *)malloc(sizeof(char *) * 500);
    strcpy(jokePointer, "REG|");
    tempLength = (char *)malloc(sizeof(char *) * 500);
    sprintf(tempLength, "%d", (int)strlen(punchLine));
    strcat(jokePointer, tempLength);
    strcat(jokePointer, "|");
    strcat(jokePointer, punchLine);
    strcat(jokePointer, "|");
    jokePointer[strlen(jokePointer)] = '\0';
    //send the punch line to the client
    write(c->fd, jokePointer, strlen(jokePointer));
    printf("%s\n", punchLine);

    tmTr = 0;
    bzero(tm, 100);
    pipeCount = 0;
    contentLength = atoi(tempLength);
    lengthString = (char *) malloc(sizeof(char *) * 10);
    lengthStringTracker = 0;

    while (1)
    {
        bzero(buf, 100);
        if (pipeCount < 2)
        {
            nread = read(c->fd, buf, 1);
            if(pipeCount == 1 && buf[0] != '|') {
                if(!isdigit(buf[0])) {
                    write(c->fd, "ERR|M5LN|", strlen("ERR|M5LN|"));
                    printf("%s\n", errMessage[16]);
                    close(c->fd);
                    free(c);
                    return NULL;
                }
                lengthString[lengthStringTracker] = buf[0];
                lengthStringTracker++;
            }
            tm[tmTr] = buf[0];
            tmTr++;
        }
        else
        {
            lengthString[lengthStringTracker] = '\0';
            nread = read(c->fd, buf, contentLength + 1);
            int j = 0;
            for (j = 0; j < (contentLength + 1); j++)
            {
                tm[tmTr] = buf[j];
                tmTr++;
            }
        }
        if (nread > 0)
        {
            buf[nread] = '\0';
            pipeCount += countPipes(buf);
            if (pipeCount == 3)
            {
                break;
            }
        }
        else {
            printf("%s\n", errMessage[17]);
            break;
        }
    }
    tm[tmTr] = '\0';
    tempErrorString = (char *)malloc(sizeof(char *) * ((int)strlen(tm) + 5));
    int tmp = 0;
    int i = 0;
    tracker = 0;
    for (i = 0; i < tmTr; i++)
    {
        if (tracker == 2 && tm[i] != '|')
        {
            tempErrorString[tmp] = tm[i];
            tmp++;
        }
        if (tm[i] == '|')
            tracker++;
    }
    tempErrorString[tmp] = '\0';
    errValue = errorChecker(tm, tempErrorString, tmp, 3);
    if (errValue == 0)
    {
        write(c->fd, "ERR|M5CT|", strlen("ERR|M5CT|"));
        printf("%s\n", errMessage[15]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 1)
    {
        write(c->fd, "ERR|M5LN|", strlen("ERR|M5LN|"));
        printf("%s\n", errMessage[16]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 2)
    {
        write(c->fd, "ERR|M5FT|", strlen("ERR|M5FT|"));
        printf("%s\n", errMessage[17]);
        close(c->fd);
        free(c);
        return NULL;
    }
    else if (errValue == 10)
    {
        printf("%s\n", tm);
        close(c->fd);
        free(c);
        return NULL;
    }
    printf("%s\n", tempErrorString);
    free(lengthString);
    lengthStringTracker = 0;

    close(c->fd);
    free(c);
    return NULL;
}
