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
#define LOGOUT "1"

int main (int argc, char **argv)
{
    char client_queue_name [64];
    char client_username [64];
    mqd_t qd_server, qd_client;   // queue descriptors


    // create the client queue for receiving messages from server
    sprintf (client_queue_name, "/user-%d", getpid ());

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Client: mq_open (client)");
        exit (1);
    }

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit (1);
    }

    char in_buffer [MSG_BUFFER_SIZE];
    char out_buffer [MSG_BUFFER_SIZE];

    printf ("Porfavor ingrese su nombre de usuario: ");
    fgets (client_username, 64, stdin);
    sprintf(out_buffer, "%s&%s&%s", client_queue_name, CREATE, client_username);

    // send message to server
    if (mq_send (qd_server, out_buffer, strlen (out_buffer), 0) == -1) {
        perror ("Client: Not able to send message to server");
        //continue;
    }

    if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Client: mq_receive");
            exit (1);
    }
    printf ("Bienvenido: %s", client_username);

    while(1){
        char* option;
        fgets (option, 64, stdin);

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

    if (mq_unlink (client_queue_name) == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }
    printf ("Client: bye\n");

    exit (0);
}