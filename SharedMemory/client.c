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

struct client
{
    //char queue_name[64];
    char option[64];
    char username[64];
    char destino[64];
    char mensaje[MAX_MSG_SIZE];
};

struct client request;

int main (int argc, char **argv)
{
    //mqd_t qd_server, qd_client;
    char in_buffer [MSG_BUFFER_SIZE];
    char out_buffer [MSG_BUFFER_SIZE];
    //struct mq_attr attr;

    //sprintf (request.queue_name, "/user-%d", getpid());

    //attr.mq_flags = 0;
    //attr.mq_maxmsg = MAX_MESSAGES;
    //attr.mq_msgsize = MAX_MSG_SIZE;
    //attr.mq_curmsgs = 0;

    //Opens client queue descriptor
    /*if ((qd_client = mq_open (request.queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Client: mq_open (client)");
        exit (1);
    }

    //Opens server queue descriptor
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit (1);
    }*/

    int fd_client = shm_open("SharedMemory", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_client == -1){
      printf("shm_open");
      return 0;
    }

    char* addr = mmap(NULL, MSG_BUFFER_SIZE*2, PROT_READ | PROT_WRITE, MAP_SHARED, fd_client, 0);
    if (addr == MAP_FAILED){
      printf("mmap");
      return 0;
    }


    printf ("Porfavor ingrese su nombre de usuario: ");
    getInput(request.username);
    sprintf(out_buffer, "%s&%s&", request.username, CREATE);
    memset(addr, 0, MSG_BUFFER_SIZE);
    strncpy(addr, out_buffer, MSG_BUFFER_SIZE);
    memset(out_buffer, 0, MSG_BUFFER_SIZE);
    // send login request
    /*if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
        perror ("Client: Not able to send message to server");
    }else{
        memset(out_buffer, 0, MSG_BUFFER_SIZE);
    }*/


    //receive confirmation
    sleep(1);
    memset(in_buffer, 0, MSG_BUFFER_SIZE);
    strncpy(in_buffer, &(addr[MSG_BUFFER_SIZE]), MSG_BUFFER_SIZE);
    char* serverResponse;
    if(!decode(in_buffer, &serverResponse)){
        printf ("Lo sentimos %s, ha occurrido un error :/\n", request.username);
        return 0;
    }
    /*if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Client: mq_receive");
            exit (1);
    }else{*/
    if((strcmp(serverResponse, OK))){
        printf("%s\n", serverResponse);
        return 0;
    }
    //}
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

            sprintf(out_buffer, "%s&%s&%s&%s&%s&", request.username, request.option, request.destino, request.mensaje);

            memset(addr, 0, MSG_BUFFER_SIZE);
            strncpy(addr, out_buffer, MSG_BUFFER_SIZE);
            memset(out_buffer, 0, MSG_BUFFER_SIZE);
            // send request
            /*if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
                perror ("Client: Not able to send message to server");
            }else{
                memset(out_buffer, 0, MSG_BUFFER_SIZE);
            }*/

            sleep(1);
            memset(in_buffer, 0, MSG_BUFFER_SIZE);
            strncpy(in_buffer, &(addr[MSG_BUFFER_SIZE]), MSG_BUFFER_SIZE);
            if(!decode(in_buffer, &serverResponse)){
                printf("%s\n", "POrque estas entrando aca?");
                continue;
            }
            /*
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
            }*/

            if((strcmp(serverResponse, OK))){
                printf("%s\n", serverResponse);
            }else
                printf("%s\n", "Mensaje enviado con exito!");
        }/*else if(!strcmp(request.option, READ)){
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
        }*/
    }

    if (close(fd_client) == -1){
      printf("close");
      return 0;
    }
    /*if (mq_close (qd_client) == -1) {
        perror ("Client: mq_close");
        exit (1);
    }

    if (mq_unlink (request.queue_name) == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }*/
    printf ("STATUS: Closed\n");

    exit (0);
}

int getInput(char* target){
    fgets (target, MAX_MSG_SIZE, stdin);
    target = strtok(target, "\n");
    return 1;
}

int decode(char* input, char** output){
    char* inputUsername = strtok(input, "&");
    *output = strtok(NULL, "&");

    if(!strcmp(inputUsername, request.username))
        return 1;
    return 0;
}