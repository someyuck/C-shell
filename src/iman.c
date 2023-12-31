#include "headers.h"

#define buffer_size 10000 * 1000

void iman(char **args, int num_args)
{
    if (num_args == 2)
    {
        // handle DNS resolution
        char *host = "man.he.net"; // web host
        char *port = "80"; // HTTTP port
        // URL will be in format : /?topic=<argument>&section=all 
        char *url = (char *)malloc(sizeof(char) * (strlen("/?topic=") + strlen(args[1]) + strlen("&section=all") + 1));
        strcpy(url, "/?topic=");
        strcat(url, args[1]);
        strcat(url, "&section=all");

        int sock;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints)); // clear the struct
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        struct addrinfo *result_LL, *resultPtr;

        // open a socket and report error if it fails
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            fprintf(stderr, "\033[1;31mERROR: failed to open a socket\033[0m\n");
            return;
        }

        int ret = getaddrinfo(host, port, &hints, &result_LL);
        if (ret != 0)
        {
            fprintf(stderr, "\033[1;31mERROR: getaddrinfo: %s\033[0m\n", gai_strerror(ret));
            return;
        }

        // create a TCP socket to the server, by trying each one of the addresses returned by getaddrinfo()
        resultPtr = result_LL;
        while (resultPtr != NULL)
        {
            sock = socket(resultPtr->ai_family, resultPtr->ai_socktype, resultPtr->ai_protocol);
            if (sock == -1)
                continue;
            if (connect(sock, resultPtr->ai_addr, resultPtr->ai_addrlen) == 0)
                break;
            else
                close(sock);
            resultPtr = resultPtr->ai_next;
        }

        freeaddrinfo(result_LL);

        if (resultPtr == NULL) //  couldn't connect to any address
        {
            fprintf(stderr, "\033[1;31mERROR: failed to connect\033[0m\n");
            close(sock);
            free(url);
            return;
        }

        // send a get request
        // string format: "GET /<url> HTTP/1.1\r\nHost: <hostname>\r\n\r\n"
        char *req_string = (char *)malloc(sizeof(char) * (strlen("GET /") + strlen(url) + strlen(" HTTP/1.1\r\nHost: ") + strlen(host) + strlen("\r\n\r\n") + 1));
        strcpy(req_string, "GET /");
        strcat(req_string, url);
        strcat(req_string, " HTTP/1.1\r\nHost: ");
        strcat(req_string, host);
        strcat(req_string, "\r\n\r\n");

        if (send(sock, req_string, strlen(req_string), 0) == -1)
        {
            fprintf(stderr, "\033[1;31mERROR: failed to send GET request\033[0m\n");
            close(sock);
            free(url);
            free(req_string);
            return;
        }

        // now receive response from the server
        char *read_buf = (char *)malloc(sizeof(char) * (buffer_size));
        int total_read = 0;
        int bytes_read = 0;
        while (total_read < buffer_size - 1)
        {
            bytes_read = recv(sock, read_buf + total_read, buffer_size - total_read - 1, 0); // to append at the buffer's end instead of overwriting
            if (bytes_read == 0)
                break;
            else if (bytes_read == -1)
            {
                fprintf(stderr, "\033[1;31mERROR: server did not send any response\033[0m\n");
                close(sock);
                free(url);
                free(req_string);
                free(read_buf);
                return;
            }
            total_read += bytes_read;
        }
        if (total_read == buffer_size - 1)
            fprintf(stderr, "\033[1;31mThe buffer has been fully occupied. Printing whatever has been received\033[0m\n");

        read_buf[total_read - 1] = '\0';

        // close the socket
        close(sock);

        // now, print the message after processing
        skip_HTML_and_print(read_buf);

        free(url);
        free(read_buf);
        free(req_string);
    }
    else
        fprintf(stderr, "\033[1;31mERROR: iMan: invalid syntax\033[0m\n");
}

void skip_HTML_and_print(char *input)
{
    // find the second occurrence of "\n\n" -- that's where the actual content (with NAME) starts.
    // from there print till the last occurence of "\n\n" -- that's where the actual content ends, with only a small footer left
    char *first = strstr(input, "\n\n");
    if (first == NULL)
    {
        printf("\033[1;31mERROR: No such process found\033[0m\n");
        return;
    }
    char *second = strstr(first + 2, "\n\n");
    if (second == NULL)
    {
        printf("\033[1;31mERROR: No such process found\033[0m\n");
        return;
    }

    char *last = second + 2;
    char *temp;
    while (last != NULL)
    {
        temp = last;
        last = strstr(last, "\n\n");
        if (last != NULL)
            last += 2; // skip the found "\n\n"
    }
    last = temp - 2; // dont want the last "\n\n" to be printed

    *last = '\0'; // so that footer etc are not printed
    printf("%s\n\n", second + 2); // as the NAME field immediately follows the second "\n\n"
    *last = 'x'; // dummy character so free works later.
}