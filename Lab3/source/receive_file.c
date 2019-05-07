#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "receive_file.h"
#include "sockwrap.h"

int extern buffer_size;
int extern long_output;

int client_receive_file_from_server(int , char *);
int for_receive(int,int,char*);

int client_receive_file_from_server(int socket, char *file_name)
{
    int secTimer = 15, res_sel;
    unsigned long int i;
    char *buf, buffer[buffer_size],buf5[5],buf4[4];
    FILE *file;
    uint32_t dimension, timestamp;
    struct timeval tval;

    //inizializzazione timer
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(socket, &cset);
    tval.tv_sec = secTimer;
    tval.tv_usec = 0;

    if (send(socket, "GET ", 4, MSG_NOSIGNAL) != 4)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: GET' ' inviato\n");
    if (send(socket, file_name, strlen(file_name), MSG_NOSIGNAL) != strlen(file_name))
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: nome file inviato\n");
    if (send(socket, "\r\n", 2, MSG_NOSIGNAL) != 2)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: CR LF inviato\n");
    printf("- RICHIESTO FILE '%s' -\n", file_name);
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        if (recv(socket, buf5, 5, 0) != 5)
        {
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        return (-1);
    }
    if (buf5[0] == '-')
    {
        printf("- ERRORE RICHIESTA -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        exit(1);
    }
    if (strncmp(buf5, "+OK\r\n", 5) != 0)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: +OK ricevuto\n");

    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        if (recv(socket, buf4, 4, 0) != 4)
        {
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        return (-1);
    }
    if (buf4[0] == '-')
    {
        printf("- ERRORE RICHIESTA -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        exit(1);
    }
    dimension = ntohl((*(uint32_t *)buf4));
    if (long_output)
        printf("PASS: dimensione '%u' Byte ricevuta\n", dimension);
    file = fopen(file_name, "w");
    if (file == NULL)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }

    if (long_output)
        printf("PASS: file creato\n");
    printf("- RICEZIONE IN CORSO -\n");
    i = 0;
    while(1)
    {
        if((dimension - i) > buffer_size)
        {
            if(for_receive(socket,buffer_size,buffer) == -1)
            {
                if (long_output)
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            if (fwrite(buffer, 1, buffer_size, file) < 0)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            i += buffer_size;
        }
        else
        {
            buf = malloc((dimension - i)*sizeof(char));
            if(for_receive(socket,(dimension - i),buf) == -1)
            {
                if (long_output)
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            if (fwrite(buf, 1, (dimension - i), file) < 0)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            //free(buf);
            break;
        }
    }
    if (fclose(file) != 0)
    {
        if (long_output)
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: file scritto\n");

    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        if (recv(socket, buf4, 4, 0) != 4)
        {
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 1, __FILE__);
            return (-1);
        }
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        return (-1);
    }
    if (buf4[0] == '-')
    {
        printf("- ERRORE RICHIESTA -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        exit(1);
    }
    if (long_output)
        printf("PASS: timestamp ricevuto\n");
    timestamp = ntohl((*(uint32_t *)buf4));
    time_t ts = timestamp;
    struct tm *timestamp_format;
    timestamp_format = localtime(&ts);
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
    printf("- RICEVUTO FILE: %s -\n", buffer);

    return (1);
}

int for_receive(int socket,int dimension,char *buf)
{
    unsigned long int i,received_byte=0;

    for(received_byte=0;received_byte!=dimension;)
    {
        if((i = recv(socket, buf, dimension-received_byte, 0)) == -1)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        received_byte += i;
    }
    return(0);
}

