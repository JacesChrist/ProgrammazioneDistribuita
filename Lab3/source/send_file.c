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

int extern buffer_size;
int extern long_output;

int server_send_file_to_client(int);
void serverSendErr(int);

int server_send_file_to_client(int socket)
{
    int secTimer = 15, res_sel, ultima_modifica;
    unsigned long int dimension,i,j,sent_byte;
    char nome_file[50], buffer[buffer_size], buf4[4];
    FILE *file;
    uint32_t size, timestamp;
    struct stat stats;
    struct timeval tval;
    //inizializzazione timer
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(socket, &cset);
    tval.tv_sec = secTimer;
    tval.tv_usec = 0;
    //timer
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: timer inizializzato\n");
    if (res_sel > 0)
    {
        //ricezione G E T ' '
        if (recv(socket, buf4, 4, 0) != 4)
        {
            close(socket);
            printf("- CONNESSIONE CHIUSA -\n");
            return (0);
        }
        if (strncmp(buf4, "GET ", 4) != 0)
        {
            serverSendErr(socket);
            return (0);
        }
        if (long_output) printf("PASS: GET' ' ricevuto\n");
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        serverSendErr(socket);
        close(socket);
        return (0);
    }
    i = 0;
    while (1)
    {
        //timer
        res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
        if (res_sel == -1)
        {
            serverSendErr(socket);
            return (0);
        }
        if (long_output) printf("PASS: timer inizializzato\n");
        if (res_sel > 0)
        {
            //ricezione nome file
            recv(socket, &nome_file[i], 1, 0);
            if (nome_file[i] == 13)
            {
                nome_file[i] = '\0';
                if (long_output) printf("PASS: nome file ricevuto\n");
                //timer
                res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
                if (res_sel == -1)
                {
                    serverSendErr(socket);
                    return (0);
                }
                if (long_output) printf("PASS: timer inizializzato\n");
                if (res_sel > 0)
                {
                    //ricezione (CR) LF
                    recv(socket, &nome_file[i + 1], 1, 0);
                    if (nome_file[i + 1] != 10)
                    {
                        serverSendErr(socket);
                        return (0);
                    }
                    if (long_output) printf("PASS: CR LF ricevuti\n");
                    break;
                }
                else
                {
                    printf("- TIMEOUT CONNESSIONE -\n");
                    serverSendErr(socket);
                    close(socket);
                    return (0);
                }
            }
            else
            {
                i++;
                if (i == buffer_size)
                {
                    serverSendErr(socket);
                    return (0);
                }
            }
        }
        else
        {
            printf("- TIMEOUT CONNESSIONE -\n");
            serverSendErr(socket);
            close(socket);
            return (0);
        }
    }
    printf("- RICHIESTO FILE '%s' -\n", nome_file);
    //apertura file
    file = fopen(nome_file, "r");
    if (file == NULL)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: file aperto\n");
    //invio + O K
    if (send(socket, "+OK\r\n", 5, MSG_NOSIGNAL) != 5)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: +OK CR LF inviato\n");
    //conteggio dimensione
    stat(nome_file, &stats);
    dimension = stats.st_size;
    size = htonl(dimension);
    if (send(socket, &size, 4, MSG_NOSIGNAL) != 4)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: dimensione '%lu' Byte inviata\n", dimension);
    //scansione-invio file
    printf("- INVIO IN CORSO -\n");
    for(sent_byte = 0;sent_byte != dimension;)
    {
        if((dimension - sent_byte) >= buffer_size)
        {
            if(fread(buffer,1,buffer_size,file) != buffer_size)
            {
                serverSendErr(socket);
                return (0);
            }
            for(i=0;i!=buffer_size;)
            {
                if((j = send(socket, buffer, (buffer_size-i), MSG_NOSIGNAL)) == -1)
                {
                    serverSendErr(socket);
                    return (-1);
                }
                i += j;
            }
            sent_byte += buffer_size;
        }
        else
        {
            if(fread(buffer,1,(dimension - sent_byte),file) != (dimension - sent_byte))
            {
                serverSendErr(socket);
                return (0);
            }
            for(i=0;i!=(dimension - sent_byte);)
            {
                if((j = send(socket, buffer, (buffer_size-sent_byte-i), MSG_NOSIGNAL)) == -1)
                {
                    serverSendErr(socket);
                    return (-1);
                }
                i += j;
            }
            sent_byte += (dimension-sent_byte);
        }
    }
    if (fclose(file) != 0) return(0);
    printf("- FILE INVIATO -\n");
    //invio timestamp
    ultima_modifica = stats.st_mtime;
    timestamp = htonl(ultima_modifica);
    if (send(socket, &timestamp, 4, MSG_NOSIGNAL) != 4)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output)
    {
        struct tm *timestamp_format;
        time_t ts = ultima_modifica;
        timestamp_format = localtime(&ts);
        strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
        printf("PASS: timestamp '%s' inviato\n", buffer);
    }
    return (1);
}

void serverSendErr(int socket_error)
{
    printf("- ERRORE NELLA PROCEDURA -\n");
    if (send(socket_error, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6) return;
    if (close(socket_error) != 0) return;
    return;
}
