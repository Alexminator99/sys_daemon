#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "file_log.h"
#include <systemd/sd-journal.h>

#define LOG_FILE_PATH "/etc/sec-info/file.conf"

int main()
{
    // create a daemon process and run it in the background
    time_t daemon_time;
    daemon_time = time(NULL);

    printf("Initializing deamon running... \n");

    pfile file;
    
    init_file_info(&file, LOG_FILE_PATH);
    fflush(NULL);
    
    sd_journal_print(LOG_NOTICE, "Deamon initialized... \n");

    while (1)
    {
        /* Codigo del servicio */
        fflush(NULL);
        sd_journal_print(LOG_NOTICE,"Daemon running... \n");
        chk_files(&file);
        sd_journal_print(LOG_NOTICE,"Daemon running at:%s", (char *)ctime(&daemon_time));
        sleep(SLEEP_TIME_CHECK_SEC);
    }

    

    return 0;
}
