#include "fcgi_stdio.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


void main(void)
{

    int count = 0;
    while(FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }
        printf("Content-type: text/html\r\n"
               "\r\n"
               "<title>FastCGI Hello!</title>"
               "<h1>FastCGI Hello!</h1>"
               "Request number %d running on host <i>%s</i>\n",
                ++count, getenv("SERVER_NAME"));

        printf("<br />CONTENT_LENGTH = %d <br />\r\n", len);
        printf("<form enctype='multipart/form-data' method='post' action='?'><input type='text' name='text1' /><input type='file' name='file1'/><input type='submit' /></form>");
        printf("<hr />");

        fflush(stdout);

        FCGI_FILE * fileOut = FCGI_fopen("/tmp/fcgi.out", "w");
        if (fileOut) {
            int done = 0;
            while(done < len) {
                char buffer[1024];
                int i;
                int packetRead;

                packetRead = FCGI_fread(buffer, 1, sizeof(buffer), stdin);
                if (packetRead < 0) {
                    break;
                }
                if (packetRead > 0) {
                    FCGI_fwrite(buffer, 1, packetRead, fileOut);
                    done += packetRead;
                }


            }
            FCGI_fclose(fileOut);
        }

        FCGI_Finish();
    }
}