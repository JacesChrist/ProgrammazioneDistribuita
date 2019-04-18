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
#include "errlib.h"

int extern buffer_size;
int extern long_output;

int client_receive_file_from_server(int socket, char *file_name)
{
    int secTimer = 15, res_sel;
    char *buf, buffer[buffer_size];
    unsigned long received_byte;
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
    //sleep(10);
    if (send(socket, file_name, strlen(file_name), MSG_NOSIGNAL) != strlen(file_name))
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: nome file inviato\n");
    //sleep(10);
    if (send(socket, "\r\n", 2, MSG_NOSIGNAL) != 2)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS: CR LF inviato\n");
    printf("- RICHIESTO FILE '%s' -\n", file_name);
    buf = malloc(5 * sizeof(char));
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
        if (recv(socket, buf, 5, 0) != 5)
        {
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
        {
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        return (-1);
    }
    if (buf[0] == '-')
    {
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
            recv(socket, buf, 1, 0);
        }
        else
        {
            printf("- TIMEOUT CONNESSIONE -\n");
            if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            close(socket);
            return (-1);
        }
        printf("- ERRORE RICHIESTA -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        exit(1);
    }
    if (strncmp(buf, "+OK\r\n", 5) != 0)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    free(buf);
    if (long_output)
        printf("PASS: +OK ricevuto\n");

    buf = malloc(4 * sizeof(char));
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
        if (recv(socket, buf, 4, 0) != 4)
        {
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
        {
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        return (-1);
    }

    if (buf[0] == '-')
    {
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
            recv(socket, buf, 2, 0);
        }
        else
        {
            printf("- TIMEOUT CONNESSIONE -\n");
            if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            if (close(socket) != 0)
            {
                if (long_output)
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            return (-1);
        }
        printf("- ERRORE RICHIESTA -\n");
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        exit(1);
    }
    dimension = ntohl((*(uint32_t *)buf));
    free(buf);
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
    received_byte = 0;
    while (1)
    {
        if ((dimension - received_byte) > buffer_size)
        {
            res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
            if (res_sel == -1)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            if (res_sel > 0)
            {
                if (recv(socket, buffer, buffer_size, 0) != buffer_size)
                {
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                    return (-1);
                }
            }
            else
            {
                printf("- TIMEOUT CONNESSIONE -\n");
                if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
                {
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                    return (-1);
                }
                if (close(socket) != 0)
                {
                    if (long_output)
                        printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                    return (-1);
                }
                return (-1);
            }
            if (fprintf(file, "%s", buffer) < 0)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            received_byte += buffer_size;
        }
        else
        {
            buf = malloc((dimension - received_byte) * sizeof(char));
            res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
            if (res_sel == -1)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            if (res_sel > 0)
            {
                if (recv(socket, buf, (dimension - received_byte), 0) != (dimension - received_byte))
                {
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                    return (-1);
                }
            }
            else
            {
                printf("- TIMEOUT CONNESSIONE -\n");
                if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
                {
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                    return (-1);
                }
                if (close(socket) != 0)
                {
                    if (long_output)
                        printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                    return (-1);
                }
                return (-1);
            }
            if (fprintf(file, "%s", buf) < 0)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            free(buf);
            break;
        }
    }
    if (fclose(file) != 0)
    {
        if (long_output)
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
        return (-1);
    }
    free(buf);
    if (long_output)
        printf("PASS: file scritto\n");

    buf = malloc(4 * sizeof(char));
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
        if (recv(socket, buf, 4, 0) != 4)
        {
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 1, __FILE__);
            return (-1);
        }
    }
    else
    {
        printf("- TIMEOUT CONNESSIONE -\n");
        if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
        {
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
            return (-1);
        }
        if (close(socket) != 0)
        {
            if (long_output)
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        return (-1);
    }
    if (buf[0] == '-')
    {
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
            recv(socket, buf, 2, 0);
        }
        else
        {
            printf("- TIMEOUT CONNESSIONE -\n");
            if (send(socket, "-ERR\r\n", 6, MSG_NOSIGNAL) != 6)
            {
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
                return (-1);
            }
            if (close(socket) != 0)
            {
                if (long_output)
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (-1);
            }
            return (-1);
        }
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
    timestamp = ntohl((*(uint32_t *)buf));
    free(buf);
    time_t ts = timestamp;
    struct tm *timestamp_format;
    timestamp_format = localtime(&ts);
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
    printf("- ULTIMA MODIFICA FILE: %s -\n", buffer);

    return (1);
}