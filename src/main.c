#define _GNU_SOURCE
#include <stdio.h>
#include "fcgi_stdio.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include "multipart_parser.h"

int on_part_data(multipart_parser *parser, const char *data, size_t length)
{
#if 0
    printf("Content-type: text/html\r\n\r\n<html><body>got data</body></html>\r\n");
    fflush(stdout);
#endif
    FCGI_FILE * fileOut = FCGI_fopen("/var/cache/firmware-hi3518.bin", "a");
    if (fileOut)
    {
        FCGI_fwrite((void *)data, 1, length, fileOut);
        FCGI_fclose(fileOut);
    }
    
    return 0;
}

int on_part_data_end(multipart_parser *parser)
{
    system("mv /var/cache/firmware-hi3518.bin /var/cache/firmware/");

    return 0;
}

int main(void)
{
    while(FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        char *org_boundary = getenv("CONTENT_TYPE");
#if 0
        if (org_boundary)
        {
            printf("Content-type: text/html\r\n\r\n<html><body>%s</body></html>\r\n", org_boundary);
        }
        else
        {
            printf("Content-type: text/html\r\n\r\n<html><body>not found boundary</body></html>\r\n");
        }
        fflush(stdout);
#endif        
        org_boundary = strstr(org_boundary, "boundary=");
        if (org_boundary)
        {
            char *boundary = NULL;
            int len;

            asprintf(&boundary, "--%s", org_boundary + strlen("boundary="));
            if (boundary)
            {
                if (contentLength != NULL)
                {
                    len = strtol(contentLength, NULL, 10);
                }
                else
                {
                    len = 0;
                }

                int done = 0;
                char *buffer = malloc(len);
                if (buffer)
                {
                    while(done < len)
                    {
                        int packetRead;

                        packetRead = FCGI_fread(buffer + done, 1, len - done, stdin);
                        if (packetRead < 0)
                        {
                            break;
                        }
                        if (packetRead > 0)
                        {
                            done += packetRead;
                        }
                    }
                    multipart_parser_settings callbacks;
                    memset(&callbacks, 0, sizeof(multipart_parser_settings));
                    callbacks.on_part_data = on_part_data;
                    callbacks.on_part_data_end = on_part_data_end;
                    multipart_parser* parser = multipart_parser_init(boundary, &callbacks);
                    multipart_parser_execute(parser, buffer, len);
                    multipart_parser_free(parser);
#if 0
                    printf("Content-type: text/html\r\n\r\n<html><body>%s<br>%s</body></html>\r\n", boundary, buffer);
                    fflush(stdout);
#endif
                    free(buffer);
                }
                free(boundary);
            }
        }
        
        FCGI_Finish();
    }

    return 0;
}
