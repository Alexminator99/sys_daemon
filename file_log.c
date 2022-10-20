#include "file_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <systemd/sd-journal.h>

int files_to_process(char *files)
{
    printf("Getting files in: %s \n", files);

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

    printf("Files to process: %d \n", count);

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
    printf("User ID: %d for file %s", attr.st_uid, file);
    return attr.st_uid;
}

void init_file_info(pfile file_info, char *files)
{
    // Initialize file info
    printf("Initializing file structure info with %s \n", files);
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
        printf("File Path is %s \n", line);
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
    printf("\n\n");
    printf("--------------------------------------------------------------\n");
    printf("|Total files to check: %d                                     |\n", file_info->tot_arch);
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < file_info->tot_arch; i++)
    {
        printf("|File: %-52s  |", file_info->arch[i]);
        printf("\r\n");

        char time[50];
        strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&file_info->time_mod[i]));

        printf("|Last modified time: %s                     |", time);
        printf("\r\n");
        printf("|User ID: %.4d                                               |", file_info->user_mod[i]);
        printf("\r\n");
        printf("--------------------------------------------------------------");
        printf("\n");
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
            printf("MESSAGE_ID=52fb62f99e2c49d89cfbf9d6de5e3555\n");
            printf("File %.23s was modified by user with ID: %d", file_info->arch[i], file_info->user_mod[i]);
            printf("\n");
            printf( "---------------------------------------------");
            printf("\n");
        }
    }

    if (files_were_modified == 0)
    {
        printf("\n\n");
        printf("----------------------------------");
        printf("\n");
        printf("|     Files were not modified    |");
        printf("\n");
        printf("----------------------------------");
        printf("\n");
    }
}
