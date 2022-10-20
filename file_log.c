#include "file_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <systemd/sd-journal.h>

int files_to_process(char *files)
{
    sd_journal_print(LOG_NOTICE,"Getting files in: %s \n", files);

    FILE *fp;
    int count = 0;
    char *line = NULL;
    size_t size = 0;
    ssize_t read;

    fp = fopen(files, "r");
    if (fp == NULL)
    {
        perror("Fail while opening config file");
        exit(EXIT_FAILURE);
    }

    // Cuenta el numero de lineas del archivo
    while ((read = getline(&line, &size, fp)) != -1)
    {
        if (read != 0)
            ++count;
    }

    free(line);
    fclose(fp);

    sd_journal_print(LOG_NOTICE, "Files to process: %d \n", count);

    return count;
}

time_t fmod_status(char *file)
{
    // Obtain last modification date of a file
    struct stat attr;
    stat(file, &attr);
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attr.st_mtime));
    return attr.st_mtime;
}

uid_t user_status(char *file)
{
    // Obtain user id of a file
    struct stat attr;
    stat(file, &attr);
    sd_journal_print(LOG_NOTICE,"User ID: %d for file %s", attr.st_uid, file);
    return attr.st_uid;
}

void init_file_info(pfile file_info, char *files)
{
    // Initialize file info
    sd_journal_print(LOG_NOTICE,"Initializing file structure info with %s \n", files);
    file_info->tot_arch = files_to_process(files);
    file_info->arch = (char **)malloc(file_info->tot_arch * sizeof(char *));

    // Open file in LOG_FILE_PATH and read each line and store it in pfile->arch
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(files, "r");
    if (fp == NULL)
    {
        return;
    }

    for (int i = 0; i < file_info->tot_arch; i++)
    {
        read = getline(&line, &len, fp);
        file_info->arch[i] = (char *)malloc(sizeof(char) * read);
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        sd_journal_print(LOG_NOTICE,"File Path is %s \n", line);
        strcpy(file_info->arch[i], line);
    }

    fclose(fp);
    free(line);

    // Get last modification date for each file in pfile->arch
    file_info->time_mod = (time_t *)malloc(file_info->tot_arch * sizeof(time_t));
    // Get user id for each file in pfile->arch
    file_info->user_mod = (uid_t *)malloc(file_info->tot_arch * sizeof(uid_t));
    // Update pfile->check_mod flag if it was modified
    file_info->check_mod = (int *)malloc(file_info->tot_arch * sizeof(int));
    for (int i = 0; i < file_info->tot_arch; i++)
    {
        file_info->time_mod[i] = fmod_status(file_info->arch[i]);
        file_info->user_mod[i] = user_status(file_info->arch[i]);
        file_info->check_mod[i] = 0;
    }

    // Print file info
    sd_journal_print(LOG_NOTICE,"\n\n");
    sd_journal_print(LOG_NOTICE,"--------------------------------------------------------------\n");
    sd_journal_print(LOG_NOTICE,"|Total files to check: %d                                     |\n", file_info->tot_arch);
    sd_journal_print(LOG_NOTICE,"--------------------------------------------------------------\n");
    for (int i = 0; i < file_info->tot_arch; i++)
    {
        sd_journal_print(LOG_NOTICE,"|File: %-52s  |", file_info->arch[i]);
        sd_journal_print(LOG_NOTICE,"\r\n");

        char time[50];
        strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&file_info->time_mod[i]));

        sd_journal_print(LOG_NOTICE,"|Last modified time: %s                     |", time);
        sd_journal_print(LOG_NOTICE,"\r\n");
        sd_journal_print(LOG_NOTICE,"|User ID: %.4d                                               |", file_info->user_mod[i]);
        sd_journal_print(LOG_NOTICE,"\r\n");
        sd_journal_print(LOG_NOTICE,"--------------------------------------------------------------");
        sd_journal_print(LOG_NOTICE,"\n");
    }
}

void chk_files(pfile file_info)
{
    // Check if files were modified
    int files_were_modified = 0;
    for (int i = 0; i < file_info->tot_arch; i++)
    {
        if (difftime(file_info->time_mod[i], fmod_status(file_info->arch[i])))
        {
            file_info->check_mod[i] = 1;
            file_info->time_mod[i] = fmod_status(file_info->arch[i]);

            files_were_modified = 1;

            // If file was modified, journal file was modified plus user id
            sd_journal_print(LOG_NOTICE,"MESSAGE_ID=52fb62f99e2c49d89cfbf9d6de5e3555\n");
            sd_journal_print(LOG_NOTICE,"File %.23s was modified by user with ID: %d", file_info->arch[i], file_info->user_mod[i]);
            sd_journal_print(LOG_NOTICE,"\n");
            sd_journal_print(LOG_NOTICE, "---------------------------------------------");
            sd_journal_print(LOG_NOTICE,"\n");
        }
    }

    if (files_were_modified == 0)
    {
        sd_journal_print(LOG_NOTICE,"\n\n");
        sd_journal_print(LOG_NOTICE,"----------------------------------");
        sd_journal_print(LOG_NOTICE,"\n");
        sd_journal_print(LOG_NOTICE,"|     Files were not modified    |");
        sd_journal_print(LOG_NOTICE,"\n");
        sd_journal_print(LOG_NOTICE,"----------------------------------");
        sd_journal_print(LOG_NOTICE,"\n");
    }
}

