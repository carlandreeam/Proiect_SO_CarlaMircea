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
#include <sys/wait.h>
#include <stdint.h>

typedef struct
{
    unsigned char rosu;
    unsigned char verde;
    unsigned char albastru;
} RGB;

void close_file(int fd2)
{
    if (close(fd2) == -1)
    {
        perror("error close file\n");
    }
}

int creat_file(char *path)
{
    int fd_out = creat(path, S_IWUSR | S_IRUSR);
    if (fd_out == -1)
    {
        perror("error creat");
        exit(1);
    }

    close_file(fd_out);

    fd_out = open(path, O_RDWR);
    if (fd_out == -1)
    {
        perror("error creat");
        exit(1);
    }

    return fd_out;
}

int numarare_linii(int fd, char *file)
{
    struct stat st_file;
    lseek(fd, 0, SEEK_SET);
    if (stat(file, &st_file) == -1)
    {
        perror("stat error");
        exit(1);
    }

    uint8_t c;
    int read_int;
    int cnt = 0;

    while ((read_int = read(fd, &c, sizeof(uint8_t))) != 0)
    {
        if (read_int == -1)
        {
            perror("error read");
            exit(1);
        }

        if (c == '\n')
        {
            cnt++;
        }
    }

    return cnt;
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

void conv_img(char *file, struct stat st_file)
{

    int fd = open(file, O_RDWR);
    if (fd == -1)
    {
        perror("error open");
        exit(1);
    }

    char header[54];
    if (read(fd, header, sizeof(header)) != sizeof(header))
    {
        perror("error read");
        exit(1);
    }
    int lungime = *(int *)&header[18];
    int inaltime = *(int *)&header[22];

    uint16_t bitCount = *(uint16_t *)&header[28];

    lseek(fd, 0, SEEK_SET);

    if (write(fd, header, sizeof(header)) != sizeof(header))
    {
        perror("error write");
        exit(1);
    }

    if (bitCount == 24)
    {
        RGB color;
        char buffer[200];

        for (int i = 0; i < lungime * inaltime; i++)
        {

            if (read(fd, &color.rosu, sizeof(uint8_t)) != sizeof(uint8_t) || read(fd, &color.verde, sizeof(uint8_t)) != sizeof(uint8_t) || read(fd, &color.albastru, sizeof(uint8_t)) != sizeof(uint8_t))
            {
                perror("error read");
                exit(1);
            }

            uint8_t gri = color.rosu * 0.3 + color.verde * 0.5 + color.albastru * 0.2;

            lseek(fd, 54 + i * 3, SEEK_SET);
            strcpy(buffer, "");
            sprintf(buffer, "%c%c%c", gri, gri, gri);
            afisare(fd, buffer);
        }
    }
    else if (bitCount <= 8)
    {
        RGB colors[256];
        for (int i = 0; i < 256; i++)
        {
            if (read(fd, &colors[i].rosu, sizeof(uint8_t)) != sizeof(uint8_t) || read(fd, &colors[i].verde, sizeof(uint8_t)) != sizeof(uint8_t) || read(fd, &colors[i].albastru, sizeof(uint8_t)) != sizeof(uint8_t))
            {
                perror("error read");
                exit(1);
            }
            colors[i].rosu = colors[i].verde = colors[i].albastru = i;
        }

        lseek(fd, 54, SEEK_SET);
        if (write(fd, colors, sizeof(colors)) != sizeof(colors))
        {
            perror("error write");
            exit(1);
        }
    }

    if (close(fd) == -1)
    {
        perror("error close");
        exit(1);
    }
}

void bmp(char *file, struct stat st_file, int fd_out)
{
    char buffer[200];

    int fd = open(file, O_RDWR);
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
    sprintf(buffer, "inaltime: %d\n", inaltime);
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

    sprintf(buffer, "drepturi de acces user legatura: ");
    drepturi(st_link, "user", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces grup legatura: ");
    drepturi(st_link, "grup", buffer);
    afisare(fd_out, buffer);

    sprintf(buffer, "drepturi de acces altii legatura: ");
    drepturi(st_link, "altii", buffer);
    afisare(fd_out, buffer);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("incorrect number of arguments");
        exit(1);
    }
    printf("parinte %d\n", getpid());
    pid_t pid;
    int status;

    char director_in[200];
    char director_out[200];
    strcpy(director_in, argv[1]);
    strcpy(director_out, argv[2]);
    DIR *dir_in = opendir(director_in);
    if (dir_in == NULL)
    {
        perror("open director in");
        exit(1);
    }
    DIR *dir_out = opendir(director_out);
    if (dir_out == NULL)
    {
        perror("open director out");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir_in)) != NULL)
    {

        char *file = entry->d_name;
        char path[300];
        char buffer[300];
        sprintf(buffer, "nume fisier: %s\n", file);
        sprintf(path, "%s/%s", director_in, file);

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

        char file_out[300];
        strcpy(file_out, director_out);
        strcat(file_out, "/");
        strcat(file_out, file);
        strcat(file_out, "_statistica.txt");

        if (S_ISDIR(st_file.st_mode))
        {
            if (strcmp(file, ".") != 0 && strcmp(file, "..") != 0)
            {
                pid = fork();
                if (pid < 0)
                {
                    perror("error fork");
                    exit(-1);
                }
                else if (pid == 0)
                {

                    int fd_out = creat_file(file_out);

                    afisare(fd_out, buffer);
                    director(st_file, fd_out);
                    afisare(fd_out, "\n");
                    int nr_linii = numarare_linii(fd_out, file_out);

                    exit(nr_linii);
                    if (close(fd_out) == -1)
                    {
                        perror("close file");
                        exit(1);
                    }
                }
            }
        }
        else if (S_ISLNK(st_link.st_mode) && S_ISREG(st_file.st_mode))
        {
            pid = fork();
            if (pid < 0)
            {
                perror("error fork");
                exit(-1);
            }
            else if (pid == 0)
            {
                int fd_out = creat_file(file_out);

                afisare(fd_out, buffer);
                legatura(st_file, st_link, fd_out);
                afisare(fd_out, "\n");
                int nr_linii = numarare_linii(fd_out, file_out);

                exit(nr_linii);
                if (close(fd_out) == -1)
                {
                    perror("close file");
                    exit(1);
                }
            }
        }
        else if (S_ISREG(st_file.st_mode))
        {
            pid = fork();
            if (pid < 0)
            {
                perror("error fork");
                exit(-1);
            }
            else if (pid == 0)
            {
                int fd_out = creat_file(file_out);

                afisare(fd_out, buffer);
                if (strstr(entry->d_name, ".bmp") != NULL)
                {
                    bmp(path, st_file, fd_out);
                    pid_t pid_convert = fork();
                    if (pid_convert < 0)
                    {
                        perror("error fork for image conversion");
                        exit(-1);
                    }
                    else if (pid_convert == 0)
                    {
                        conv_img(path, st_file);
                        exit(0);
                    }
                    waitpid(pid_convert, NULL, 0);
                }
                else
                {
                    fisier_obisnuit(st_file, fd_out);
                }
                afisare(fd_out, "\n");
                int nr_linii = numarare_linii(fd_out, file_out);
                exit(nr_linii);
                if (close(fd_out) == -1)
                {
                    perror("close file");
                    exit(1);
                }
            }
        }
    }
    while ((pid = wait(&status)) != -1)
        if (WIFEXITED(status))
        {
            printf("Child with pid = %d ended with status %d\n", pid, WEXITSTATUS(status));
        }

    if (closedir(dir_in) == -1)
    {
        perror("close director");
        exit(1);
    }
    if (closedir(dir_out) == -1)
    {
        perror("close director");
        exit(1);
    }

    return 0;
}