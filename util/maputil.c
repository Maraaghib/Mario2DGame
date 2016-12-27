#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>

#include "error.h"

/* Cette fonction renvoie la largeur de la carte dont le chemin est filename*/
unsigned getwidth(char* filename){
	int fd, r;
	unsigned width;

	fd = open(filename, O_RDONLY);
	if (fd == -1){
		exit_with_error("Opening error !\n");
	}

	// Lecture de la largeur de la carte
	r = read(fd, &width, sizeof(unsigned));
	if(r == -1){
		exit_with_error("Reading error !\n");
	}

	close(fd);
	return width;
}

/* Cette fonction renvoie la hauteur de la carte dont le chemin est filename*/
unsigned getheight(char* filename){
	int fd, r, pos;
	unsigned height;

	fd = open(filename, O_RDONLY);
	if (fd == -1){
		exit_with_error("Opening error !\n");
	}

	// Avance le curseur pour lire la hauteur
	pos = lseek(fd, sizeof(unsigned), SEEK_SET);
	if (pos == -1){
		exit_with_error("lseekn");
	}

	// Lecture de la largeur de la carte
	r = read(fd, &height, sizeof(unsigned));
	if(r == -1){
		exit_with_error("Reading error !\n");
	}

	close(fd);
	return height;
}

/* Cette fonction renvoie le nomnbre de type d'objets de la carte dont le chemin est filename*/
int getobjects(char* filename){
	int fd, r, pos, cur = 0;
	int nb_objects;

	fd = open(filename, O_RDONLY);
	if (fd == -1){
		exit_with_error("Opening error !\n");
	}

	// Avance le curseur pour lire le nombre d'objets
	cur += sizeof(unsigned) + sizeof(unsigned);
	pos = lseek(fd, cur, SEEK_SET);
	if (pos == -1){
		exit_with_error("lseek\n");
	}

	// Lecture du nombre d'objects de la carte
	r = read(fd, &nb_objects, sizeof(int));
	if(r == -1){
		exit_with_error("Reading error !\n");
	}

	close(fd);
	return nb_objects;
}

/* Cette fonction renvoie le nomnbre de d'éléments (objets) de la carte dont le chemin est filename*/
int getelements(char* filename){
	int fd, r, pos, cur = 0;
	int nb_elements;

	fd = open(filename, O_RDONLY);
	if (fd == -1){
		exit_with_error("Opening error !\n");
	}

	// Avance le curseur pour lire le nombre d'elements contenus dans la carte
	cur += sizeof(unsigned) + sizeof(unsigned) + sizeof(int);
	pos = lseek(fd, cur, SEEK_SET);
	if (pos == -1){
		exit_with_error("lseek\n");
	}

	// Lecture du nombre d'elements de la carte
	r = read(fd, &nb_elements, sizeof(int));
	if(r == -1){
		exit_with_error("Reading error !\n");
	}

	close(fd);
	return nb_elements;
}

/* Cette fonction renvoie une chaîne de caractère contenant la date et l'heure
   de la dernière modification de la carte dont le chemin est filename*/
char* getdate(char* filename){
	char* date = malloc(30 * sizeof(char));
	struct stat s; //inscrit la date de deniere modif du fichier 

	if(stat(filename,&s) == -1){
		exit_with_error("stat\n");
	}
	struct tm *dt; // Pointeur vers une structure contenant la date et l'heure
	time_t time_modif = s.st_mtime; // Heure dernière modification
	dt = localtime(&time_modif); //convertit time_modif en fonction du fuseau horaire de l'utilisateur
	sprintf(date, "Le %d/%d/%d à %02u:%02u:%02u\n", dt->tm_mday, dt->tm_mon, 1900 + dt->tm_year, dt->tm_hour, dt->tm_min, dt->tm_sec);
	return date;
}

/* Cette fonction affiche les infos concernant une carte dont le chemin est filename */
void getinfo(char* filename){
	printf("La largeur de la carte est: %u\n", getwidth(filename));
	printf("La hauteur de la carte est: %u\n", getheight(filename));
	printf("Le nombre de types d'objets de la carte est: %d\n", getobjects(filename));
	printf("Le nombre d'éléments (objets) de la carte est: %d\n", getelements(filename));
	printf("La date de dernière modification est: %s\n", getdate(filename));
	free(getdate(filename));
}

void setwidth(char* filename){

}

void setheight(char* filename){
	
}


int main(int argc, char const *argv[])
{
	if(argc < 3){
		exit_with_error("At least three parameters !\nUsage: ./maputil <file> --option\n");
	}

	// Copie caractère par caractère du 2ème argument à la variable filename
	char* filename = (char*)malloc(strlen(argv[1]) * sizeof(char));
	if(filename == NULL) exit_with_error("Allocation failed !");

	for (int i = 0; i < strlen(argv[1]); ++i){
		filename[i] = argv[1][i];
	}

	int val;
    const char* optstring = "";
    while (1) {
        int option_index = 0; // index du tableau ci-dessous
        // Tableau de structures struct option déclarées dans <getopt.h>
        static struct option long_options[] =
        {
            {"getwidth", no_argument, NULL, 'w'},
            {"getheight", no_argument, NULL, 'h'},
            {"getobjects", no_argument, NULL, 'o'},
            {"getelements", no_argument, NULL, 'e'},
            {"getdate", no_argument, NULL, 'd'},
            {"getinfo", no_argument, NULL, 'i'},
            {NULL, 0, NULL, 0}
        };
        // getopt_long permet d'analyser les options (longues) de la ligne de commande
        val = getopt_long(argc, (char * const*)argv, optstring, long_options, &option_index);
        
        switch(val){ // Suivant les valeurs renvoyées par getopt_long
            case 'w':
                printf("La largeur de la carte est: %u\n", getwidth(filename));
                if (optarg) // S'il y a un paramètre de l'option getwidth indexée par 'w'
                    exit(EXIT_FAILURE);
                break;
            case 'h':
                printf("La hauteur de la carte est: %u\n", getheight(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'o':
                printf("Le nombre de types d'objets de la carte est: %d\n", getobjects(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'e':
                printf("Le nombre d'éléments (objets) de la carte est: %d\n", getelements(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'd':
            	printf("La date de dernière modification est: %s\n", getdate(filename));
				free(getdate(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'i':
            	getinfo(filename);
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            default: exit(EXIT_FAILURE);
            break;
        }
        if (optind < argc) {
            printf("Arguments ne constituant pas des options : ");
            while (optind < argc)
            printf("%s (optind = %d)", argv[optind++], (optind-1));
            printf("\n");
        }
    }

	return EXIT_SUCCESS;;
}
