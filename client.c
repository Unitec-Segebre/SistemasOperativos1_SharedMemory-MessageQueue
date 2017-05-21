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

struct client
{
    char queue_name[64];
    char option[64];
    char username[64];
    char destino[64];
    char mensaje[MSG_BUFFER_SIZE];
};

int main (int argc, char **argv)
{
    mqd_t qd_server, qd_client;
    char in_buffer [MSG_BUFFER_SIZE];
    char out_buffer [MSG_BUFFER_SIZE];
    //char client_queue_name [64];
    //char client_username [64];
    struct client request;
    struct mq_attr attr;

    sprintf (request.queue_name, "/user-%d", getpid());

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    //Opens client queue descriptor
    if ((qd_client = mq_open (request.queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Client: mq_open (client)");
        exit (1);
    }

    //Opens server queue descriptor
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit (1);
    }


    printf ("Porfavor ingrese su nombre de usuario: ");
    getInput(request.username);
    sprintf(out_buffer, "%s&%s&%s&", request.queue_name, CREATE, request.username);

    // send login request
    if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
        perror ("Client: Not able to send message to server");
        //continue;
    }else{
        memset(out_buffer, 0, MSG_BUFFER_SIZE);
    }


    //receive confirmation
    memset(in_buffer, 0, MSG_BUFFER_SIZE);
    if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Client: mq_receive");
            exit (1);
    }else{
        if((strcmp(in_buffer, OK))){
            printf("%s\n", in_buffer);
            return 0;
        }
    }
    printf ("Bienvenido: %s\n", request.username);

    while(1){
        /*
        log out
        */
        getInput(request.option);
        if(!strcmp(request.option, LOGOUT)){
            printf("Ten un buen dia %s!\n", request.username);
            return 0;
        }else if(!strcmp(request.option, SEND)){
            printf ("Destino: ");
            getInput(request.destino);
            printf ("Mensaje: ");
            getInput(request.mensaje);
            //printf("Destino: %s\nMensaje: %s\n", request.destino, request.mensaje);

            sprintf(out_buffer, "%s&%s&%s&%s&%s&", request.queue_name, request.option, request.username, request.destino, request.mensaje);

            // send request
            if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
                //continue;
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }
            memset(in_buffer, 0, MSG_BUFFER_SIZE);
            if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
                    perror ("Client: mq_receive");
                    exit (1);
            }else{
                if((strcmp(in_buffer, OK))){
                    printf("%s\n", in_buffer);
                }
                else
                    printf("%s\n", "Mensaje enviado con exito!");
            }

        }

    }

    /*char temp_buf [10];

    while (fgets (temp_buf, 2, stdin)) {

        // send message to server
        if (mq_send (qd_server, client_queue_name, strlen (client_queue_name), 0) == -1) {
            perror ("Client: Not able to send message to server");
            continue;
        }

        // receive response from server

        if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Client: mq_receive");
            exit (1);
        }
        // display token received from server
        printf ("Client: Token received from server: %s\n\n", in_buffer);

        printf ("Ask for a token (Press ): ");
    }*/


    if (mq_close (qd_client) == -1) {
        perror ("Client: mq_close");
        exit (1);
    }

    if (mq_unlink (request.queue_name) == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }
    printf ("Client: bye\n");

    exit (0);
}

int getInput(char* target){
    fgets (target, MAX_MSG_SIZE, stdin);
    target = strtok(target, "\n");
    return 1;
}