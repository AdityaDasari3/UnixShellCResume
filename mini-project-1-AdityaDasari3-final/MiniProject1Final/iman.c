#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define PORT 80

void strip_html_tags(char* str) {
    int in_tag = 0;
    char* dest = str;
    for (char* src = str; *src; src++) {
        if (*src == '<') {
            in_tag = 1;
        } else if (*src == '>') {
            in_tag = 0;
        } else if (!in_tag) {
            *dest++ = *src;
        }
    }
    *dest = '\0';
}

void fetch_man_page(const char *command) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE] = {0};
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE * 10] = {0};  // Larger buffer for entire response

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    server = gethostbyname("man.he.net");
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        return;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    bcopy((char *)server->h_addr_list[0], 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return;
    }

    // Prepare the GET request
    snprintf(request, sizeof(request), 
             "GET /?topic=%s&section=all HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n", command);

    // Send the request
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Failed to send request");
        close(sock);
        return;
    }

    // Read the response
    int total_bytes = 0;
    char* response_ptr = response;
    while (1) {
        int valread = read(sock, buffer, BUFFER_SIZE - 1);
        if (valread <= 0) break;
        
        memcpy(response_ptr, buffer, valread);
        response_ptr += valread;
        total_bytes += valread;
        
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(sock);

    // Extract content between <PRE> tags
    char* start = strstr(response, "<PRE>");
    char* end = strstr(response, "</PRE>");
    
    if (start && end) {
        start += 5;  // Move past "<PRE>"
        *end = '\0';  // Null-terminate at end of content
        
        strip_html_tags(start);
        
        // Print each line, trimming whitespace
        char* line = strtok(start, "\n");
        while (line) {
            // Trim leading whitespace
            while (isspace((unsigned char)*line)) line++;
            
            // Trim trailing whitespace
            char* end = line + strlen(line) - 1;
            while (end > line && isspace((unsigned char)*end)) end--;
            *(end + 1) = '\0';
            
            if (*line) {  // Only print non-empty lines
                printf("%s\n", line);
            }
            line = strtok(NULL, "\n");
        }
    } else {
        printf("Man page content not found for '%s'\n", command);
    }
}