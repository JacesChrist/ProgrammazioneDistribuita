#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>

#include "receive_file.h"
#include "sockwrap.h"
#include "errlib.h"

#define buffer_size 500

int extern long_output;

int server_send_file_to_client(int);
void serverSendErr(int);

int server_send_file_to_client(int socket)
{
    int secTimer = 5, ultima_modifica;
    unsigned long int i, dimension, sent_byte, status_bar1, status_bar2;
    char nome_file[50], *buf, buffer[buffer_size];
    FILE *file;
    uint32_t size, timestamp;
    struct stat stats;

    //inizializzazione timer
    /*fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(socket, &set);
    timeout.tv_sec = secTimer;
    timeout.tv_sec = 0;
    if (long_output)
        printf("PASS timer inizializzato\n");*/

    //if (select(FD_SETSIZE, &set, NULL, NULL, &timeout))
    {
        //ricezione G E T ' '
        buf = malloc(4 * sizeof(char));
        if (recv(socket, buf, 4, 0) != 4)
        {
            free(buf);
            close(socket);
            return (-1);
        }
        if (strncmp(buf, "GET ", 4) != 0)
        {
            serverSendErr(socket);
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (1);
        }
        free(buf);
        if (long_output)
            printf("PASS GET' ' ricevuto\n");
    }
    /*else
    {
        printf("- TIMEOUT CONNESSIONE -");
        fflush(stdout);
    }*/

    i = 0;
    while (1)
    {
        //ricezione nome file
        recv(socket, &nome_file[i], 1, 0);
        if (nome_file[i] == 13)
        {
            nome_file[i] = '\0';
            if (long_output)
                printf("PASS nome file ricevuto\n");
            //ricezione (CR) LF
            recv(socket, &nome_file[i + 1], 1, 0);
            if (nome_file[i + 1] != 10)
            {
                serverSendErr(socket);
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (1);
            }
            if (long_output)
                printf("PASS CR LF ricevuti\n");
            break;
        }
        else
        {
            i++;
            if (i == buffer_size)
            {
                serverSendErr(socket);
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (1);
            }
        }
    }

    printf("- RICHIESTO FILE '%s' -\n", nome_file);
    //invio + O K
    if (send(socket, "+OK\r\n", 5, MSG_NOSIGNAL) != 5)
    {
        printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS +OK CR LF inviato\n");
    //conteggio dimensione
    stat(nome_file, &stats);
    dimension = stats.st_size;
    size = htonl(dimension);
    send(socket, &size, 4, MSG_NOSIGNAL);
    if (long_output)
        printf("PASS dimensione '%lu' Byte inviata\n", dimension);
    //apertura file
    file = fopen(nome_file, "r");
    if (file == NULL)
    {
        serverSendErr(socket);
        printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS file aperto\n");
    //scansione-invio file
    status_bar1 = dimension / 10;
    status_bar2 = 0;
    sent_byte = 0;
    printf("- INVIO IN CORSO ");
    while (1)
    {
        if ((dimension - sent_byte) > buffer_size)
        {
            if (fread(buffer, buffer_size, 1, file) != 1)
            {
                serverSendErr(socket);
                printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            if (send(socket, buffer, buffer_size, MSG_NOSIGNAL) != buffer_size)
            {
                serverSendErr(socket);
                printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            sent_byte += buffer_size;
        }
        else
        {
            buf = malloc((dimension - sent_byte) * sizeof(char));
            if (fread(buf, (dimension - sent_byte), 1, file) != 1)
            {
                serverSendErr(socket);
                printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            if (send(socket, buf, (dimension - sent_byte), MSG_NOSIGNAL) != (dimension - sent_byte))
            {
                serverSendErr(socket);
                printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            free(buf);
            break;
        }
        //barra di aggiornamento
        if (sent_byte >= status_bar2)
        {
            printf("#");
            status_bar2 += status_bar1;
            fflush(stdout);
        }
    }
    printf(" -\n");
    if (long_output)
        printf("PASS file inviato\n");

    //invio timestamp
    fclose(file);
    ultima_modifica = stats.st_mtime;
    timestamp = htonl(ultima_modifica);
    if (send(socket, &timestamp, 4, MSG_NOSIGNAL) != 4)
    {
        printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
        return (-1);
    }
    if (long_output)
    {
        struct tm *timestamp_format;
        time_t ts = ultima_modifica;
        timestamp_format = localtime(&ts);
        strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
        printf("PASS timestamp '%s' inviato\n", buffer);
    }

    return (1);
}

void serverSendErr(int socket_error)
{
    if (send(socket_error, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return;
    }
    return;
}