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
#define READ "ver mensajes"
#define CLEAR "borrar mensajes"
#define OK "OK"

struct client
{
    char queue_name[64];
    char option[64];
    char username[64];
    char destino[64];
    char mensaje[MAX_MSG_SIZE];
};

int main (int argc, char **argv)
{
    mqd_t qd_server, qd_client;
    char in_buffer [MSG_BUFFER_SIZE];
    char out_buffer [MSG_BUFFER_SIZE];
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
        printf("%s", "CMD> ");
        getInput(request.option);
        if(!strcmp(request.option, LOGOUT)){
            printf("Ten un buen dia %s!\n", request.username);
            break;
        }else if(!strcmp(request.option, SEND)){
            printf ("Destino: ");
            getInput(request.destino);
            printf ("Mensaje: ");
            getInput(request.mensaje);

            sprintf(out_buffer, "%s&%s&%s&%s&%s&", request.queue_name, request.option, request.username, request.destino, request.mensaje);

            // send request
            if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
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
        }else if(!strcmp(request.option, READ)){
            sprintf(out_buffer, "%s&%s&%s&", request.queue_name, request.option, request.username);
            if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
                //continue;
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }
            while(1){
                memset(in_buffer, 0, MSG_BUFFER_SIZE);
                if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
                        perror ("Client: mq_receive");
                        continue;
                }else{
                    if(!strcmp(in_buffer, OK)){
                        break;
                    }
                    else{
                        char* from = strtok(in_buffer, "&");
                        char* mensaje = strtok(NULL, "&");
                        printf("Mensaje de %s: %s\n", from, mensaje);
                        memset(from, 0, strlen(from));
                        memset(mensaje, 0, strlen(mensaje));
                    }

                }
            }
        }else if(!strcmp(request.option, CLEAR)){
            sprintf(out_buffer, "%s&%s&%s&", request.queue_name, request.option, request.username);
            if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }

            if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
                    perror ("Client: mq_receive");
                    exit (1);
            }else{
                if((strcmp(in_buffer, OK))){
                    printf("%s\n", in_buffer);
                }
                else
                    printf("%s\n", "Mensajes Borrados!");
            }
        }else{
            printf("Comando \"%s\" no es soportado.\nComandos soportados: enviar mensaje, ver mensajes, borrar mensajes, cerrar sesion\n", request.option);
        }
    }
    if (mq_close (qd_client) == -1) {
        perror ("Client: mq_close");
        exit (1);
    }

    if (mq_unlink (request.queue_name) == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }
    printf ("STATUS: Closed\n");

    exit (0);
}

int getInput(char* target){
    fgets (target, MAX_MSG_SIZE, stdin);
    target = strtok(target, "\n");
    return 1;
}