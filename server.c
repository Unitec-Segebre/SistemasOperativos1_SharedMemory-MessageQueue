#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_QUEUE_NAME   "/server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

#define CREATE "0"
#define LOGOUT "cerrar sesion"
#define SEND "enviar mensaje"
#define OK "OK"

struct in_request
{
    char* user_id;
    char* request_id;
    char* username;
    mqd_t qd_id;
    char* destino;
    char* mensaje;
};

struct user
{
    mqd_t qd_id;
    char* username;
    char* inbox[MAX_MSG_SIZE];
};

struct in_request decode(char* input);
int isUserExists(char* tryUsername);
struct user* all_users;// = (struct user*)malloc(MAX_MESSAGES*sizeof(struct user));//Asumiendo que solo recibo un request por usuario
mqd_t qd_server;
char* in_buffer;
char* out_buffer;

int main (int argc, char **argv)
{

    int i, j;
    //Initialize all_users
    all_users = (struct user*)malloc(MAX_MESSAGES*sizeof(struct user));
    for (i = 0; i < MAX_MESSAGES; i++)
    {
        all_users[i].qd_id = -1;
        all_users[i].username = (char*)malloc(64);
        for(j = 0; j < MAX_MESSAGES; j++){
            all_users[i].inbox[j] = (char*)malloc(MAX_MSG_SIZE);
            memset(all_users[i].inbox[j], 0, MAX_MSG_SIZE);
        }
    }

    printf ("Server: Up and Running Pollo's mail services!\n");

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit (1);
    }

    in_buffer = (char*)malloc(MSG_BUFFER_SIZE*sizeof(char));// [MSG_BUFFER_SIZE];
    out_buffer = (char*)malloc(MSG_BUFFER_SIZE*sizeof(char));// [MSG_BUFFER_SIZE];

    struct in_request user_request;
    static const struct in_request user_request_EmptyStruct;


    while (1) {
        memset(in_buffer, 0, MSG_BUFFER_SIZE);
        if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Server: mq_receive");
            exit (1);
        }

        user_request = user_request_EmptyStruct;
        user_request = decode(in_buffer);
        //printf("ID: %s\n", user_request.user_id);
        //printf("Request id:%s\n", user_request.request_id);
        //printf("Username: %s\n", user_request.username);
        //printf("Queue descriptor: %d\n", user_request.qd_id);
        //printf("Destino: %s\n", user_request.destino);
        //printf("Mensaje%s\n", user_request.mensaje);
        if(!strcmp(user_request.request_id, CREATE)){
            int userIndex = isUserExists(user_request.username);
            if( userIndex == -1){
                int indexNewUser = findNextAvailableUsers();
                printf("STATUS: New user - %s@%d\n", user_request.username, indexNewUser);
                if(indexNewUser == -1){
                    printf("%s\n", "STATUS: Server Full!");
                    sprintf(out_buffer, "%s", "Server Full, could not create connection!\n"); //Client side: when received doesnt do anything but continue
                }else{
                    all_users[indexNewUser].qd_id = user_request.qd_id;
                    sprintf(all_users[indexNewUser].username, "%s", user_request.username);
                    sprintf(out_buffer, "%s", OK);
                }
            }else{
                all_users[userIndex].qd_id = user_request.qd_id;
                sprintf(out_buffer, "%s", OK);
            }
            if (mq_send (user_request.qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Server: Not able to send message to client");
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }
        }else if(!strcmp(user_request.request_id, SEND)){
            int destinoInUsers = isUserExists(user_request.destino);
            if(destinoInUsers != -1){
                int targetInbox = findNextAvailableInbox(all_users[destinoInUsers]);
                if(targetInbox == -1){
                    sprintf(out_buffer, "No se pudo mandar mensaje, el usuario \"%s\" no puede recibir mas mensajes!", user_request.destino);
                    if (mq_send (all_users[isUserExists(user_request.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                        perror ("Server: Not able to send message to client");
                    }else{
                        memset(out_buffer, 0, MSG_BUFFER_SIZE);
                    }
                }else{
                    strcpy(all_users[destinoInUsers].inbox[targetInbox], user_request.mensaje);
                    sprintf(out_buffer, "%s", OK);
                    if (mq_send (all_users[isUserExists(user_request.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                        perror ("Server: Not able to send message to client");
                    }else{
                        memset(out_buffer, 0, MSG_BUFFER_SIZE);
                    }
                }

            }else{
                sprintf(out_buffer, "No se pudo mandar mensaje, el usuario \"%s\" no existe!", user_request.destino);
                if (mq_send (all_users[isUserExists(user_request.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                    perror ("Server: Not able to send message to client");
                }else{
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                }
            }
        }
        /*else if(!strcmp(user_request.request_id, LOGOUT)){
            sprintf(out_buffer, "%s", OK); //Client side: when received doesnt do anything but continue
            if (mq_send (user_request.qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Server: Not able to send message to client");
            }
        }*/
        //printf("%s\n", all_users[0].username);

        //printf ("Server: message received.\n");

        // send reply message to client

        /*if ((qd_client = mq_open (in_buffer, O_WRONLY)) == 1) {
            perror ("Server: Not able to open client queue");
            continue;
        }

        printf("%s\n", in_buffer);

        sprintf (out_buffer, "%ld", token_number);

        if (mq_send (qd_client, out_buffer, strlen (out_buffer), 0) == -1) {
            perror ("Server: Not able to send message to client");
            continue;
        }

        printf ("Server: response sent to client.\n");
        token_number++;*/


        /*
        //THIS DOESNT WORK!
        memset(user_request.user_id, 0, strlen(user_request.user_id));
        memset(user_request.request_id, 0, strlen(user_request.request_id));
        memset(user_request.username, 0, strlen(user_request.username));
        user_request.qd_id = 0;
        memset(user_request.destino, 0, strlen(user_request.destino));
        memset(user_request.mensaje, 0, strlen(user_request.mensaje));*/
    }
}

struct in_request decode(char* input){
    struct in_request in_request_data;
    in_request_data.user_id = strtok(input, "&");
    in_request_data.request_id = strtok(NULL, "&");
    in_request_data.username = strtok(NULL, "&");
    if (strcmp(in_request_data.request_id, CREATE) == 0)
    {
        if ((in_request_data.qd_id = mq_open (in_request_data.user_id, O_WRONLY)) == 1) {
            perror ("Server: Not able to open client queue");
            //continue;
        }
    }
    else if (!strcmp(in_request_data.request_id, SEND)){
        in_request_data.destino = strtok(NULL, "&");
        printf("destino: %s\n", in_request_data.destino);
        in_request_data.mensaje = strtok(NULL, "&");
        printf("mensaje: %s\n", in_request_data.mensaje);

        /*int destinoInUsers = isUserExists(in_request_data.destino);
        if(destinoInUsers != -1){
            int targetInbox = findNextAvailableInbox(all_users[destinoInUsers]);
            if(targetInbox == -1){
                sprintf(out_buffer, "No se pudo mandar mensaje, el usuario \"%s\" no puede recibir mas mensajes!", in_request_data.destino);
                if (mq_send (all_users[isUserExists(in_request_data.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                    perror ("Server: Not able to send message to client");
                }else{
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                }
            }else{
                strcpy(all_users[destinoInUsers].inbox[targetInbox], in_request_data.mensaje);
                sprintf(out_buffer, "%s", OK);
                if (mq_send (all_users[isUserExists(in_request_data.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                    perror ("Server: Not able to send message to client");
                }else{
                    memset(out_buffer, 0, MSG_BUFFER_SIZE);
                }
            }

        }else{
            sprintf(out_buffer, "No se pudo mandar mensaje, el usuario \"%s\" no existe!", in_request_data.destino);
            if (mq_send (all_users[isUserExists(in_request_data.username)].qd_id, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Server: Not able to send message to client");
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }
        }*/
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
    int i;
    for(i=0; i<MAX_MESSAGES; i++){
        if(all_users[i].qd_id == -1)
            return i;
    }
    return -1;
}

int isUserExists(char* tryUsername){
    int i;
    for(i=0; i<MAX_MESSAGES; i++){
        if(all_users[i].username != NULL && !strcmp(all_users[i].username, tryUsername)){
            printf("STATUS: Logged in! - %s\n", all_users[i].username);
            return i;
        }
    }
    return -1;
}
