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
    size_t dimension,i,sent_byte;
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
    if (long_output) printf("PASS: timer inizializzato\n");
    //timer
    res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
    if (res_sel == -1)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: timer settato\n");
    if (res_sel > 0)
    {
        //ricezione "GET "
        if (read(socket, buf4, 4) != 4)
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
        if (long_output) printf("PASS: timer settato\n");
        if (res_sel > 0)
        {
            //ricezione nome file
            read(socket, &nome_file[i], 1);
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
                if (long_output) printf("PASS: timer settato\n");
                if (res_sel > 0)
                {
                    //ricezione (CR)LF
                    read(socket, &nome_file[i + 1], 1);
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
    //invio "+OK"
    if (write(socket, "+OK\r\n", 5) != 5)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: +OK CR LF inviato\n");
    //conteggio dimensione
    stat(nome_file, &stats);
    dimension = stats.st_size;
    size = htonl(dimension);
    if (write(socket, &size, 4) != 4)
    {
        serverSendErr(socket);
        return (0);
    }
    if (long_output) printf("PASS: dimensione '%lu' Byte inviata\n", dimension);
    //loop scansione-invio file
    printf("- INVIO IN CORSO -\n");
    sent_byte = 0;
    while(sent_byte < dimension)
    {
        memset(buffer, 0, buffer_size);
        if((dimension - sent_byte) >= buffer_size)
        {
            if(fread(buffer,1,buffer_size,file) != buffer_size) return(-1);
            if(sendn(socket, buffer, buffer_size,0) != buffer_size) return(-1);
            sent_byte += buffer_size;
        }
        else
        {
            if(fread(buffer,1,(dimension - sent_byte),file) != (dimension - sent_byte)) return(-1);
            if(sendn(socket, buffer, (dimension-sent_byte),0) != (dimension - sent_byte)) return(-1);
            sent_byte += (dimension-sent_byte);
        }
    }
    if (fclose(file) != 0) return(0);
    printf("- FILE INVIATO -\n");
    //invio timestamp
    ultima_modifica = stats.st_mtime;
    timestamp = htonl(ultima_modifica);
    if (write(socket, &timestamp, 4) != 4)
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
    //terminazione con successo
    return (1);
}

void serverSendErr(int socket_error)
{
    //messaggio errore
    printf("- ERRORE NELLA PROCEDURA -\n");
    if (write(socket_error, "-ERR\r\n", 6) != 6) return;
    close(socket_error);
    return;
}
