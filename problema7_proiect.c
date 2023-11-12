#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>

void close_file(int fd2)
{
    if (close(fd2) == -1)
    {
        perror("error close file\n");
    }
}

void drepturi(struct stat st_file, char *who, char *buffer)
{
    char drepturi[200] = "";

    int depl;

    if (strcmp(who, "user") == 0)
    {
        depl = 0;
    }
    if (strcmp(who, "grup") == 0)
    {
        depl = 3;
    }
    if (strcmp(who, "altii") == 0)
    {
        depl = 6;
    }

    if ((st_file.st_mode & (S_IRUSR >> depl)) != 0)
    {
        strcat(drepturi, "R");
    }
    else
    {
        strcat(drepturi, "-");
    }

    if ((st_file.st_mode & (S_IWUSR >> depl)) != 0)
    {
        strcat(drepturi, "W");
    }
    else
    {
        strcat(drepturi, "-");
    }

    if ((st_file.st_mode & (S_IXUSR >> depl)) != 0)
    {
        strcat(drepturi, "X");
    }
    else
    {
        strcat(drepturi, "-");
    }

    strcat(drepturi, "\n");
    strcat(buffer, drepturi);
}

void afisare(int fd, char *string)
{
    if (write(fd, string, strlen(string)) == -1)
    {
        perror("error write");
        close_file(fd);
        exit(1);
    }
}

void director(struct stat st_file, int fd_out)
{
    char buffer[200];

    int id = st_file.st_uid;
    struct passwd *user = getpwuid(id);
    sprintf(buffer, "identificatorul utilizatorului: %d - %s\n", id, user->pw_name);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces user: ");
    drepturi(st_file, "user", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces grup: ");
    drepturi(st_file, "grup", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces altii: ");
    drepturi(st_file, "altii", buffer);
    afisare(fd_out, buffer);
}

void fisier_obisnuit(struct stat st_file, int fd_out)
{
    char buffer[200];

    int dimensiune = st_file.st_size;
    sprintf(buffer, "dimensiune: %d\n", dimensiune);
    afisare(fd_out, buffer);

    int id = st_file.st_uid;
    struct passwd *user = getpwuid(id);
    if (user != NULL)
    {
        sprintf(buffer, "identificatorul utilizatorului: %d - %s\n", id, user->pw_name);
    }
    else
    {
        sprintf(buffer, "identificatorul utilizatorului: %d\n", id);
    }
    afisare(fd_out, buffer);

    struct tm *timp = localtime(&st_file.st_mtime);
    if (timp != NULL)
    {
        int day = timp->tm_mday;
        int month = timp->tm_mon + 1;    // timp->tm_mon returneaza 0(ianuarie) - 11(decembrie)
        int year = timp->tm_year + 1900; // timp->tm_year returneaza anul - 1900
        sprintf(buffer, "timpul ultimei modificari: %d.%d.%d\n", day, month, year);
    }
    else
    {
        sprintf(buffer, "timpul ultimei modificari: ");
    }
    afisare(fd_out, buffer);

    int nr_legaturi = st_file.st_nlink;
    sprintf(buffer, "contorul de legaturi: %d\n", nr_legaturi);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces user: ");
    drepturi(st_file, "user", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces grup: ");
    drepturi(st_file, "grup", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces altii: ");
    drepturi(st_file, "altii", buffer);
    afisare(fd_out, buffer);
}

void bmp(char *file, struct stat st_file, int fd_out)
{
    char buffer[200];

    int fd = open(file, O_RDONLY);
    if (fd == -1)
    {
        perror("error open");
        exit(1);
    }

    int inaltime;
    lseek(fd, 22, SEEK_SET);
    if (read(fd, &inaltime, sizeof(int)) != sizeof(int))
    {
        perror("error read");
        exit(1);
    }
    sprintf(buffer, "inaltime fisier: %d\n", inaltime);
    afisare(fd_out, buffer);

    int lungime;
    lseek(fd, -8, SEEK_CUR);
    if (read(fd, &lungime, sizeof(int)) != sizeof(int))
    {
        perror("error read");
        exit(1);
    }
    sprintf(buffer, "lungime: %d\n", lungime);
    afisare(fd_out, buffer);

    fisier_obisnuit(st_file, fd_out);

    if (close(fd) == -1)
    {
        perror("error close");
        close_file(fd_out);
        exit(1);
    }
}

void legatura(struct stat st_file, struct stat st_link, int fd_out)
{
    char buffer[200];

    int dimensiune = st_link.st_size;
    sprintf(buffer, "dimensiune legatura: %d\n", dimensiune);
    afisare(fd_out, buffer);

    dimensiune = st_file.st_size;
    sprintf(buffer, "dimensiune fisier: %d\n", dimensiune);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces user: ");
    drepturi(st_link, "user", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces grup: ");
    drepturi(st_link, "grup", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces altii: ");
    drepturi(st_link, "altii", buffer);
    afisare(fd_out, buffer);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        perror("incorrect number of arguments");
        exit(1);
    }

    char director_name[200];
    strcpy(director_name,argv[1]);
    DIR *dir = opendir(director_name);
    if (dir == NULL)
    {
        perror("open director");
        exit(1);
    }

    int fd_out = creat("statistica.txt",S_IWUSR);
    if (fd_out == -1)
    {
        perror("error creat");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {

        char *file = entry->d_name;
        char path[300];
        char buffer[300];
        sprintf(buffer, "nume fisier: %s\n", file);
        sprintf(path, "%s/%s", director_name, file);

        struct stat st_file, st_link;
        if (stat(path, &st_file) == -1)
        {
            perror("stat error");
            exit(1);
        }
        if (lstat(path, &st_link) == -1)
        {
            perror("stat error");
            exit(1);
        }

        if (S_ISDIR(st_file.st_mode))
        {
            if (strcmp(file, ".") != 0 && strcmp(file, "..") != 0)
            {
                afisare(fd_out, buffer);
                director(st_file, fd_out);
                afisare(fd_out, "\n");
            }
        }
        else if (S_ISLNK(st_link.st_mode) && S_ISREG(st_file.st_mode))
        {
            afisare(fd_out, buffer);
            legatura(st_file, st_link, fd_out);
            afisare(fd_out, "\n");
        }
        else if (S_ISREG(st_file.st_mode))
        {
            afisare(fd_out, buffer);
            if (strstr(entry->d_name, ".bmp") != NULL)
            {
                bmp(path, st_file, fd_out);
            }
            else
            {
                fisier_obisnuit(st_file, fd_out);
            }
            afisare(fd_out, "\n");
        }
    }

    if (close(fd_out) == -1)
    {
        perror("close file");
        exit(1);
    }

    if (closedir(dir) == -1)
    {
        perror("close director");
        exit(1);
    }

    return 0;
}