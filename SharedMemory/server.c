#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SERVER_QUEUE_NAME   "/server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

#define CREATE "0"
#define LOGOUT "cerrar sesion"
#define SEND "enviar mensaje"
#define READ "ver mensajes"
#define CLEAR "borrar mensajes"
#define OK "OK"

struct in_request
{
    //char* user_id;
    char* username;
    char* request_id;
    //mqd_t qd_id;
    char* destino;
    char* mensaje;
};

struct user
{
    //mqd_t qd_id;
    char* username;
    char* inbox[MAX_MSG_SIZE];
};

struct in_request decode(char* input);
int isUserExists(char* tryUsername);
struct user* all_users;
int fd_server;///////////////////////
char* addr;
int nextUser;

int main (int argc, char **argv)
{

    int i, j;
    nextUser = 0;
    //Initialize all_users
    all_users = (struct user*)malloc(MAX_MESSAGES*sizeof(struct user));
    for (i = 0; i < MAX_MESSAGES; i++)
    {
        all_users[i].username = (char*)malloc(64);
        for(j = 0; j < MAX_MESSAGES; j++){
            all_users[i].inbox[j] = (char*)malloc(MAX_MSG_SIZE);
            memset(all_users[i].inbox[j], 0, MAX_MSG_SIZE);
        }
    }

    printf ("Server: Up and Running Pollo's mail services!\n");

    /*struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit (1);
    }*/

    int fd_server = shm_open("SharedMemory", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_server == -1){
        printf("shm_open");
        return 0;
    }
    if (ftruncate(fd_server, MSG_BUFFER_SIZE*2) == -1){
        printf("ftruncate");
        return 0;
    }
    addr = mmap(NULL, MSG_BUFFER_SIZE*2, PROT_READ | PROT_WRITE, MAP_SHARED, fd_server, 0);
    if (addr == MAP_FAILED){
        printf("mmap");
        return 0;
    }

    char* in_buffer = (char*)malloc(MSG_BUFFER_SIZE*sizeof(char));
    char* out_buffer = (char*)malloc(MSG_BUFFER_SIZE*sizeof(char));

    struct in_request user_request;//?
    static const struct in_request user_request_EmptyStruct;//?


    while (1) {
        if(addr[0] == 0)
            continue;
        memset(in_buffer, 0, MSG_BUFFER_SIZE);
        strncpy(in_buffer, addr, MSG_BUFFER_SIZE);
        memset(addr, 0, MSG_BUFFER_SIZE);
        /*if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Server: mq_receive");
            exit (1);
        }*/

        user_request = user_request_EmptyStruct;
        user_request = decode(in_buffer);
        if(!strcmp(user_request.request_id, CREATE)){
            int userIndex = isUserExists(user_request.username);
            if( userIndex == -1){
                int indexNewUser = findNextAvailableUsers();//
                if(indexNewUser == -1){
                    printf("%s\n", "STATUS: Server Full!");
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                    sprintf(out_buffer, "%s&%s&", user_request.username, "Server Full, could not create connection!\n"); //Client side: when received doesnt do anything but continue
                }else{
                    //all_users[indexNewUser].qd_id = user_request.qd_id;
                    printf("STATUS: New user - %s@%d\n", user_request.username, indexNewUser);
                    sprintf(all_users[indexNewUser].username, "%s", user_request.username);
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                    sprintf(out_buffer, "%s&%s&", user_request.username, OK);
                }
            }else{
                //all_users[userIndex].qd_id = user_request.qd_id;
                printf("STATUS: Logged in! - %s\n", all_users[userIndex].username);
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
                sprintf(out_buffer, "%s&%s&", user_request.username, OK);
            }
            strncpy(&(addr[MSG_BUFFER_SIZE]), out_buffer, MSG_BUFFER_SIZE);
            memset(out_buffer, 0, MSG_BUFFER_SIZE);
            /*if (mq_send (user_request.qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Server: Not able to send message to client");
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }*/
        }else if(!strcmp(user_request.request_id, SEND)){
            int destinoInUsers = isUserExists(user_request.destino);
            if(destinoInUsers != -1){
                //printf("%s\n", "Si Existe");
                int targetInbox = findNextAvailableInbox(all_users[destinoInUsers]);
                if(targetInbox == -1){
                    sprintf(out_buffer, "%s&No se pudo mandar mensaje, el usuario \"%s\" no puede recibir mas mensajes!&", user_request.username, user_request.destino);
                }else{
                    sprintf(all_users[destinoInUsers].inbox[targetInbox], "%s&%s&", user_request.username, user_request.mensaje);
                    sprintf(out_buffer, "%s&%s&", user_request.username, OK);
                }/////////////////////
                /*if (mq_send (all_users[isUserExists(user_request.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                    perror ("Server: Not able to send message to client");
                }else{
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                }*/
                memset(&(addr[MSG_BUFFER_SIZE]), 0, MSG_BUFFER_SIZE);
                strncpy(&(addr[MSG_BUFFER_SIZE]), out_buffer, MSG_BUFFER_SIZE);
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }else{
                sprintf(out_buffer, "%s&No se pudo mandar mensaje, el usuario \"%s\" no existe!&", user_request.username, user_request.destino);
                /*if (mq_send (all_users[isUserExists(user_request.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                    perror ("Server: Not able to send message to client");
                }else{
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                }*/
                memset(&(addr[MSG_BUFFER_SIZE]), 0, MSG_BUFFER_SIZE);
                strncpy(&(addr[MSG_BUFFER_SIZE]), out_buffer, MSG_BUFFER_SIZE);
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }
        /*}else if(!strcmp(user_request.request_id, READ)){
            int currentUser = isUserExists(user_request.username);
            int inboxCount = findNextAvailableInbox(all_users[currentUser]);
            for(i=0; i<inboxCount; i++){
                strcpy(out_buffer, all_users[currentUser].inbox[i]);
                if (mq_send (all_users[currentUser].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                    perror ("Client: Not able to send message to server");
                }else{
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                }
            }
            strcpy(out_buffer, OK);
            if (mq_send (all_users[currentUser].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
                //continue;
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }
        }else if(!strcmp(user_request.request_id, CLEAR)){
            int inboxCount = findNextAvailableInbox(user_request.username);
            int currentUser = isUserExists(user_request.username);
            for(i=0; i<inboxCount; i++){
                memset(all_users[currentUser].inbox[i], 0, strlen(all_users[currentUser].inbox[i]));
            }
            strcpy(out_buffer, OK);
            if (mq_send (all_users[currentUser].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
                //continue;
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }*/
        }
    }
    if (close(fd_server) == -1){
    printf("close");
    return 0;
    }
    if (shm_unlink("SharedMemory") == -1){
        printf("unlink");
        return 0;
    }
    return 0;
}

struct in_request decode(char* input){
    struct in_request in_request_data;
    in_request_data.username = strtok(input, "&");
    in_request_data.request_id = strtok(NULL, "&");
    /*if (strcmp(in_request_data.request_id, CREATE) == 0)
    {
        if ((in_request_data.qd_id = mq_open (in_request_data.user_id, O_WRONLY)) == 1) {
            perror ("Server: Not able to open client queue");
        }
    }
    else*/ if (!strcmp(in_request_data.request_id, SEND)){
        in_request_data.destino = strtok(NULL, "&");
        in_request_data.mensaje = strtok(NULL, "&");
    }
    return in_request_data;
}

int findNextAvailableInbox(struct user tryUser){
    int i;
    for(i=0; i<MAX_MESSAGES; i++){
        if(tryUser.inbox[i][0] == 0){
            return i;
        }
    }
    return -1;
}

int findNextAvailableUsers(){
    return (nextUser<10)?nextUser++:-1;
}

int isUserExists(char* tryUsername){
    int i;
    for(i=0; i<MAX_MESSAGES; i++){
        if(all_users[i].username != NULL && !strcmp(all_users[i].username, tryUsername)){
            return i;
        }
    }
    return -1;
}
