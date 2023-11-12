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

void close_files(int fd1, int fd2)
{
    if (close(fd1) == -1)
    {
        perror("error close file\n");
    }
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

void afisare(int fd, int fd_out, char *string)
{
    if (write(fd_out, string, strlen(string)) == -1)
    {
        perror("error write");
        close_files(fd, fd_out);
        exit(1);
    }
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        perror("not enough arguments");
        exit(1);
    }
    else if (argc != 2 || strstr(argv[1], ".bmp") == NULL)
    {
        char error[200];
        sprintf(error, "Usage %s %s", argv[0], argv[1]);
        perror(error);
        exit(1);
    }

    char *file = argv[1];
    int fd;
    fd = open(file, O_RDONLY);
    if (fd == -1)
    {
        perror("error open");
        exit(1);
    }

    char file_out[30] = "statistica.txt";
    int fd_out = creat(file_out, S_IRUSR | S_IWUSR);
    if (fd_out == -1)
    {
        perror("error creat");
        exit(1);
    }

    char buffer[200];

    sprintf(buffer, "nume fisier: %s\n", file);
    afisare(fd, fd_out, buffer);

    int dimensiune, lungime, inaltime;
    lseek(fd, 2, SEEK_SET);
    if (read(fd, &dimensiune, sizeof(int)) != sizeof(int))
    {
        perror("error read");
        exit(1);
    }
    lseek(fd, 12, SEEK_CUR);
    if ((read(fd, &lungime, sizeof(int)) != sizeof(int)) || (read(fd, &inaltime, sizeof(int)) != sizeof(int)))
    {
        perror("error read");
        exit(1);
    }

    sprintf(buffer, "inaltime: %d\nlungime: %d\ndimensiune: %d\n", inaltime, lungime, dimensiune);
    afisare(fd, fd_out, buffer);

    struct stat st_file;
    if (stat(file, &st_file) == -1)
    {
        perror("stat error");
        exit(1);
    }

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
    afisare(fd, fd_out, buffer);

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
    afisare(fd, fd_out, buffer);

    int nr_legaturi = st_file.st_nlink;
    sprintf(buffer, "contorul de legaturi: %d\n", nr_legaturi);
    afisare(fd, fd_out, buffer);

    sprintf(buffer, "drepturi de acces user: ");
    drepturi(st_file, "user", buffer);
    afisare(fd, fd_out, buffer);

    sprintf(buffer, "drepturi de acces grup: ");
    drepturi(st_file, "grup", buffer);
    afisare(fd, fd_out, buffer);

    sprintf(buffer, "drepturi de acces altii: ");
    drepturi(st_file, "altii", buffer);
    afisare(fd, fd_out, buffer);

    if (close(fd) == -1)
    {
        perror("error close");
        exit(1);
    }

    if (close(fd_out) == -1)
    {
        perror("error close");
        exit(1);
    }

    return 0;
}