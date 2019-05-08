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

int client_receive_file_from_server(int socket, char *file_name)
{
    int secTimer = 15, res_sel;
    unsigned long int i,j,received_byte;
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
    if (write(socket, "GET ", 4) != 4) return(-1);
    if (long_output) printf("PASS: GET' ' inviato\n");
    if (write(socket, file_name, strlen(file_name)) != strlen(file_name)) return(-1);
    if (long_output) printf("PASS: nome file inviato\n");
    if (write(socket, "\r\n", 2) != 2) return(-1);
    if (long_output) printf("PASS: CR LF inviato\n");
    printf("- RICHIESTO FILE '%s' -\n", file_name);
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1) return(-1);
    if (long_output) printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        if (read(socket, buf5, 5) != 5) return(-1);
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        close(socket);
        return (-1);
    }
    if (buf5[0] == '-')
    {
        printf("- ERRORE RICHIESTA -\n");
        close(socket);
        exit(1);
    }
    if (strncmp(buf5, "+OK\r\n", 5) != 0) return(-1);
    if (long_output) printf("PASS: +OK ricevuto\n");
    //timer
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1) return(-1);
    if (long_output) printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        if (read(socket, buf4, 4) != 4) return(-1);
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        close(socket);
        return (-1);
    }
    if (buf4[0] == '-')
    {
        printf("- ERRORE RICHIESTA -\n");
        close(socket);
        exit(1);
    }
    dimension = ntohl((*(uint32_t *)buf4));
    if (long_output) printf("PASS: dimensione '%u' Byte ricevuta\n", dimension);
    file = fopen(file_name, "w");
    if (file == NULL) return(-1);
    if (long_output) printf("PASS: file creato\n");
    printf("- RICEZIONE IN CORSO -\n");
    for(received_byte = 0;received_byte != dimension;)
    {
        memset(buffer, 0, buffer_size);
        if((dimension-received_byte) >= buffer_size)
        {
            for(i=0;i!=buffer_size;)
            {
                if((j = read(socket, buffer, (buffer_size-i))) == -1) return(-1);
                i += j;
            }
            if (fwrite(buffer, 1, buffer_size, file) < 0) return(-1);
            received_byte += buffer_size;
        }
        else
        {
            printf("aaa");
            for(i=0;i!=(dimension-received_byte);)
            {
                printf("bbb");
                if((j = read(socket, buffer,(dimension-received_byte-i))) == -1) return(-1);
                i += j;
                printf("--- %lu ---",i);
            }
            if (fwrite(buffer, 1, (dimension-received_byte), file) < 0) return(-1);
            received_byte += (dimension-received_byte);
            printf("ccc");
        }
        printf("%lu %lu\n",j,received_byte);
    }
    if (fclose(file) != 0) return(-1);
    if (long_output) printf("PASS: file scritto\n");
    //timer
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1) return(-1);
    if (long_output) printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        if (read(socket, buf4, 4) != 4) return(-1);
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        close(socket);
        return (-1);
    }
    if (buf4[0] == '-')
    {
        printf("- ERRORE RICHIESTA -\n");
        close(socket);
        exit(1);
    }
    if (long_output) printf("PASS: timestamp ricevuto\n");
    timestamp = ntohl((*(uint32_t *)buf4));
    time_t ts = timestamp;
    struct tm *timestamp_format;
    timestamp_format = localtime(&ts);
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
    printf("- RICEVUTO FILE: %s -\n", buffer);
    return (1);
}
