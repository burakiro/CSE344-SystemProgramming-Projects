/* utils.h

*/
#ifndef HEADER_FILE
#define HEADER_FILE
#define MAX 300
#define SA struct sockaddr
#define QUEUESIZE 300
#define MAX_SERVANT_LIMIT 500
typedef struct commonMessage
{
    /* Request (client --> server) */
    int typeOfMessage;
    int threadID;
    int servantPID;
    int differenceOfBounds;
    int messageWordCount;
    char requestMessage[300];
    char dataSetDirectoryNames[100][20];
    int portOfServant;
    int responseConnectionFd;
} commonMessage;

typedef struct servantInfos
{
    char dataSetDirectoryNames[100][20];
    int servantNum;
    int portOfServant;

} servantInfos;

struct node
{
    char info[20];
    struct node *next;
};

#endif
