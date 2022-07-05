#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define BACKLOG 10


int read_socket(int sock)
{
    char buffer[512];
    int bytes_read;

    bytes_read = recv(sock, buffer, sizeof(buffer), 0);
    printf("Bytes read: %d\n", bytes_read);
    buffer[bytes_read] = '\0';
    puts(buffer);
}

char *content_len_header(int con_len)
{
    char header[] = "Content-Length: ";
    int len = snprintf(NULL, 0, "%d", con_len);
    char *str = malloc(strlen(header) + len + 1);
    snprintf(str, strlen(header) + len + 1, "Content-Length: %d", con_len);
    return str;
}


char *generate_date(void)
{
    time_t t;
    t = time(NULL);

    struct tm *tm;
    tm = localtime(&t);

    char *asc_time;
    asc_time = asctime(tm);

    char date[] = "Date: ";
    char *ptr = malloc(strlen(date) + strlen(asc_time) - 1);
    strcpy(ptr, date);

    int i;
    for(i = 0; i < strlen(asc_time) - 1; i++)
    {
        ptr[i + strlen(date)] = asc_time[i];
    }
    ptr[i] = '\0';

    return ptr;
}

char *content_type(void)
{
    char str[] = "Content-Type: text/html";
    char *buf = malloc(strlen(str) + 1);

    strcpy(buf, str);
    buf[strlen(str)] = '\0';
    return buf;
}

char *connection(void)
{
    char con[] = "Connection: Closed";
    char *ptr = malloc(strlen(con) + 1);
    strcpy(ptr, con);
    ptr[strlen(con)] = '\0';
    return ptr;
}

void populate_res_header(char *http_res, int content_len)
{
    char *res_header[6];
    char status[] = "HTTP/1.1 200 OK";
    char *ptr = malloc(strlen(status) + 1);
    strcpy(ptr, status);
    ptr[strlen(status) + 1] = '\0';
    res_header[0] = ptr;
    res_header[1] = generate_date();
    res_header[2] = content_len_header(content_len);
    res_header[3] = content_type();
    res_header[4] = connection();

    char end[] = "\r\n\n";
    char end_line[] = "\n";

    strcpy(http_res, res_header[0]);
    strcat(http_res, end_line);
    strcat(http_res, res_header[1]);
    strcat(http_res, end_line);
    strcat(http_res, res_header[2]);
    strcat(http_res, end_line);
    strcat(http_res, res_header[3]);
    strcat(http_res, end_line);
    strcat(http_res, res_header[4]);
    strcat(http_res, end);



    //printf("%s\n", res_header[0]);
    //printf("%s\n", res_header[1]);
    //printf("%s\n", res_header[2]);
    //printf("%s\n", res_header[3]);
    //printf("%s\n", res_header[4]);
}

int *init_socket()
{
    int sockfd;
    struct sockaddr_in serveraddr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Exit: Socket");
        exit(1);
    }
    puts("Success: socket");

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(3000);

    const int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Exit: Bind");
        exit(1);
    }
    puts("Success: Bind");

    if(listen(sockfd, BACKLOG) < 0) 
    {
        perror("Exit: Listen");
        exit(1);
    }
    puts("Success: Server is listening");
    int *ptr = malloc(sizeof(int));
    *ptr = sockfd;
    return ptr;
}

void read_page(char read_buffer[], int *content_len)
{
    FILE *fptr;
    char c;
    if((fptr = fopen("./website.html", "r")) < 0)
    {
        perror("Exit: Open");
        exit(1);
    }

    int i = 0;
    while(fread(&c ,sizeof(char), 1, fptr) == 1)
    {
        read_buffer[i] = c;
        i++;
    }
    fclose(fptr);
    read_buffer[i] = '\0';
    *content_len = i;
}


int main (void)
{
    int *sockfd, acceptfd;
    sockfd = init_socket();

    char read_buffer[1024];
    int content_len = 0;
    read_page(read_buffer, &content_len);

    char http_res[1024];
    populate_res_header(http_res, content_len);
    strcat(http_res, read_buffer);

    while(1)
    {
        acceptfd = accept(*sockfd, NULL, NULL);
        puts("New connection accepted");
        int read_bytes = read_socket(acceptfd);

        //send(acceptfd, header, i, 0);
        send(acceptfd, http_res, 1024, 0);
        close(acceptfd);
    }
}
