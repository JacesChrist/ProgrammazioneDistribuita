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
    size_t i,received_byte;
    char buffer[buffer_size],buf5[5],buf4[4];
    uint32_t dimension, timestamp;
    struct timeval tval;
    FILE *file;

    //inizializzazione timer
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(socket, &cset);
    tval.tv_sec = secTimer;
    tval.tv_usec = 0; 
    if (long_output) printf("PASS: timer inizializzato\n");
    //invio "GET "
    if (write(socket, "GET ", 4) != 4) return(-1);
    if (long_output) printf("PASS: GET' ' inviato\n");
    if (write(socket, file_name, strlen(file_name)) != strlen(file_name)) return(-1);
    //invio nome file
    if (long_output) printf("PASS: nome file inviato\n");
    if (write(socket, "\r\n", 2) != 2) return(-1);
    if (long_output) printf("PASS: CR LF inviato\n");
    printf("- RICHIESTO FILE '%s' -\n", file_name);
    //timer
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1) return(-1);
    if (long_output) printf("PASS: timer settato\n");
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
    if (long_output) printf("PASS: timer settato\n");
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
    //creazione file
    file = fopen(file_name, "w");
    if (file == NULL) return(-1);
    if (long_output) printf("PASS: file creato\n");
    printf("- RICEZIONE IN CORSO -\n");
    //loop ricezione
    received_byte = 0;
    while(received_byte < dimension)
    {
        memset(buffer, 0, buffer_size);
        if((dimension - received_byte) >= buffer_size)
        {
            if(readn(socket,buffer,buffer_size) != buffer_size) return(-1);
            if(fwrite(buffer, 1, buffer_size, file) != buffer_size) return(-1);
            received_byte += buffer_size;
        }
        else
        {
            if(readn(socket,buffer,(dimension-received_byte)) != (dimension-received_byte)) return(-1);            
            if(fwrite(buffer, 1, (dimension-received_byte), file) != (dimension-received_byte)) return(-1);
            received_byte += (dimension-received_byte);
        }
    }
    if (fclose(file) != 0) return(-1);
    if (long_output) printf("PASS: file scritto\n");
    //timer
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1) return(-1);
    if (long_output) printf("PASS: timer settato\n");
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
    //chiusura con successo
    return (1);
}
