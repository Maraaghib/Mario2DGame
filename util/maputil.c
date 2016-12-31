#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "map.h"
#include "error.h"

/* Cette fonction permet de convertir les propriétés en nombre sous format chaîne*/
char* convertir(char const *chaine){
	if((strcmp(chaine, "air") == 0) || (strcmp(chaine, "not-destructible") == 0) || (strcmp(chaine, "not-collectible") == 0) || (strcmp(chaine, "not-generator") == 0))
		return "0"; // MAP_OBJECT_AIR

	if(strcmp(chaine, "semi-solid") == 0) 
		return "1"; // MAP_OBJECT_SEMI_SOLID

	if(strcmp(chaine, "solid") == 0)
		return "2"; // MAP_OBJECT_SOLID

	if(strcmp(chaine, "destructible") == 0)
		return "4"; // MAP_OBJECT_DESTRUCTIBLE

	if(strcmp(chaine, "collectible") == 0)
		return "8"; // MAP_OBJECT_COLLECTIBLE

	if(strcmp(chaine, "generator") == 0)
		return "16"; // MAP_OBJECT_GENERATOR

	return NULL;
}

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
		exit_with_error("Reading error (%s) !\n", filename);
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
		exit_with_error("lseek\n");
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
char* getchangedate(char* filename){
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
	printf("La date de dernière modification est: %s\n", getchangedate(filename));
	free(getchangedate(filename));
}

void setwidth(char* filename, unsigned width){
	int fd, fd_tmp, w, r, obj, abs, ord, pos, pos2, cur, cur2, old_nb_elements, nb_objects, nb_elements = 0;
	unsigned old_width, height;

	if(width < 16 || width > 1024)
		exit_with_error("Width out of range [16-1024]\n");

	// Récupération de l'ancienne largeur
	old_width = getwidth(filename);
	if(width == old_width) // Sortir de la focntion s'il y a aucune modification de la largeur
		return;

	if (fork() == 0){ // Création d'un processus fiils chargé de faire la copie
		execlp("cp", "cp", "-p", filename, "tmp.map", NULL); // Création d'un fichier temporaire, copie de filename
		_exit(EXIT_FAILURE);
	}
	wait(NULL);

	// Ouverture en lecture seule du fichier temporaire
	fd_tmp = open("tmp.map", O_RDONLY);
	if(fd_tmp == -1){
		exit_with_error("Opening error tmp!\n");
	}     

	old_nb_elements = getelements(filename); // Nombre d'éléments dans la carte avant modification
	height = getheight(filename);  // Hauteur de la carte 
	nb_objects = getobjects(filename); // Nombre de types d'objets de la carte

	// Ouverture en écriture du fichier qui doit contetnir la carte modifée
	fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if(fd == -1){
		exit_with_error("Opening error  saved!\n");
	}
	
	// Ecriture de la nouvelle largeur de la carte
	w = write(fd, &width, sizeof(unsigned));
	if(w == -1){
		exit_with_error("Writing error !\n");
	}// Ecriture de la hauteur de la carte
	w = write(fd, &height, sizeof(unsigned));
	if(w == -1){
		exit_with_error("Writing error !\n");
	}// Ecriture du nombre d'objets de la carte
	w = write(fd, &nb_objects, sizeof(int));
	if(w == -1){
		exit_with_error("Writing error !\n");
	}// Résservation dans le fichieer pour le nombre d'éléments de la carte
	w = write(fd, &nb_elements, sizeof(int));
	if(w == -1){
		exit_with_error("Writing error !\n");
	}

	cur = sizeof(unsigned) + sizeof(unsigned) + sizeof(int); // Mémoriser la postion où on doit mettre plus tard le nombre d'éléments (nb_elements)
	pos = lseek(fd_tmp, cur, SEEK_SET); // Positionne le curseur pour lire le nombre d'elements contenus dans la carte
	if (pos == -1){
		exit_with_error("lseek\n");
	}
	

	//if(width < old_width){
	cur2 = cur + sizeof(int); 
	pos2 = lseek(fd_tmp, cur2, SEEK_SET); // Se placer entre nb_elements et obj[i] dans le fichier
	if (pos2 == -1){
		exit_with_error("lseek\n");
	}

	for (int i = 0; i < old_nb_elements; ++i){
		// Lecture du numéro d'objet
		r = read(fd_tmp, &obj, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		// Lecture de l'abscisse de l'objet
		r = read(fd_tmp, &abs, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		// Lecture de l'ordonnée de l'objet
		r = read(fd_tmp, &ord, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}

		// Cas du rétrécissement: on recopie tous les éléménts de la carte compris entre 0 et width-1 (sauf le mûr côté droite) (Icf. condition de if)
		if(width < old_width){
			if((abs < width-1) || (abs == width-1 && ord == height-1)){
				nb_elements ++; // Incrémenter si un élément a pour abscisse < width-1 (width-1 sera l'abscisse du mûr de droite)
				w = write(fd, &obj, sizeof(int));
				if(w == -1){
					exit_with_error("Writing error !\n");
				}
				w = write(fd, &abs, sizeof(int));
				if(w == -1){
					exit_with_error("Writing error !\n");
				}
				w = write(fd, &ord, sizeof(int));
				if(w == -1){
					exit_with_error("Writing error !\n");
				}
			}
		}
		// Cas de l'agrandissement: on recopie tous les éléménts de la carte compris entre 0 et width-1 (sauf le mûr côté droite)
		if(width > old_width){
			if(abs == old_width-1 && ord != height-1){ // Si c'est le mûr de droite, on l'ignore, élargit la cartre puis le rajouter à la carte
				continue;
			}
			nb_elements ++; 
			w = write(fd, &obj, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error !\n");
			}
			w = write(fd, &abs, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error !\n");
			}
			w = write(fd, &ord, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error !\n");
			}
		} 
		
	}

	obj = 1; //Wall
	abs = width-1; //Abscisse mûr côté droite

	// Ecrire les objets qui constituent le mûr de droite
	for (int y = 0; y < height-1; ++y){
		nb_elements ++; 
		w = write(fd, &obj, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}
		w = write(fd, &abs, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}
		w = write(fd, &y, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}
	}

	// Remplir le "trou" du sol de l'ancienne largeur jusqu'à la nouvelle (si la nouvelle largeur est supérieure à l'anccienne)
	if(width > old_width){
		obj = 0; // Ground
		ord = height-1; // Ordonnée du sol
		for (int x = old_width; x < width; ++x){
			nb_elements++;
			w = write(fd, &obj, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error !\n");
			}
			w = write(fd, &x, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error !\n");
			}
			w = write(fd, &ord, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error !\n");
			}
		}
	}

	// Recopie des objects et leurs propriétés dans le nouveau fichier
	for (int i = 0; i < nb_objects; ++i){
		unsigned frames;
		int solidity;
		int generator;
		int collectible;
		int destructible;
		int length;

		r = read(fd_tmp, &frames, sizeof(unsigned));		
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		w = write(fd, &frames, sizeof(unsigned));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		r = read(fd_tmp, &solidity, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		w = write(fd, &solidity, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		r = read(fd_tmp, &generator, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		w = write(fd, &generator, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		r = read(fd_tmp, &collectible, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		w = write(fd, &collectible, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		r = read(fd_tmp, &destructible, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		w = write(fd, &destructible, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		r = read(fd_tmp, &length, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		w = write(fd, &length, sizeof(int)); // Ecriture de la longueur de chaque chemin dans le fichier (facilite la lecture des chaînes de caractères depuis le fichier)
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		char* path = (char*)malloc(length * sizeof(char)); // Allocation en mémoire d'un espace de stockage du chemin du fichier image
		r = read(fd_tmp, path, length);	
		if(r == -1){
			exit_with_error("Reading error !\n");
		}
		path[length] = '\0'; // Marquege de fin de chaîne
		w = write(fd, path, strlen(path));
		if(w == -1){
			exit_with_error("Writing error !\n");
		}

		path = NULL;
		free(path); // Libération de l'espace alloué
	}

	// Se positionner pour mettre à jour le nombre d'éléements qui valait 0 au départ
	pos = lseek(fd, cur, SEEK_SET);
	if (pos == -1){
		exit_with_error("lseek\n");
	}
	// Modification du nombre d'éléments de la carte
	w = write(fd, &nb_elements, sizeof(unsigned));
	if(w == -1){
		exit_with_error("Writing error !\n");
	}

	// fermeture des 2 descripteurs de fichiers et suppression du fichier temporarire
	close(fd);
	close(fd_tmp);
	remove("tmp.map");
}

void setheight(char* filename, unsigned height){
	int fd, fd_tmp, w, r, obj, abs, ord, pos, pos2, cur, cur2, old_nb_elements, nb_objects, nb_elements = 0;
	unsigned old_height, width;

	if(height < 12 || height > 20)
		exit_with_error("Height out of range [12-20]\n");

	// Récupération de l'ancienne hauteur
	old_height = getheight(filename);
	if(width == old_height) // Sortir de la focntion s'il y a aucune modification de la hauteur
		return;

	if (fork() == 0){ // Création d'un processus fils chargé de faire la copie
		execlp("cp", "cp", "-p", filename, "tmp.map", NULL); // Création d'un fichier temporaire, copie de filename
		_exit(EXIT_FAILURE);
	}
	wait(NULL);

	// Ouverture en lecture seule du fichier temporaire
	fd_tmp = open("tmp.map", O_RDONLY);
	if(fd_tmp == -1){
		exit_with_error("Opening error (tmp.map)!\n");
	}     

	old_nb_elements = getelements(filename); // Nombre d'éléments dans la carte avant modification
	width = getwidth(filename);  // Largeur de la carte 
	nb_objects = getobjects(filename); // Nombre de types d'objets de la carte

	// Ouverture en écriture du fichier qui doit contetnir la carte modifée
	fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if(fd == -1){
		exit_with_error("Opening error (%s)!\n", filename);
	}
	
	// Ecriture de la largeur de la carte
	w = write(fd, &width, sizeof(unsigned));
	if(w == -1){
		exit_with_error("Writing error (%s)!\n", filename);
	}
	// Ecriture de la nouvelle hauteur de la carte
	w = write(fd, &height, sizeof(unsigned));
	if(w == -1){
		exit_with_error("Writing error (%s)!\n", filename);
	}
	// Ecriture du nombre d'objets de la carte
	w = write(fd, &nb_objects, sizeof(int));
	if(w == -1){
		exit_with_error("Writing error (%s)!\n", filename);
	}
	// Réservation d'espace dans le fichieer pour le nombre d'éléments de la carte qui sera écrit plus tard
	w = write(fd, &nb_elements, sizeof(int));
	if(w == -1){
		exit_with_error("Writing error (%s)!\n", filename);
	}

	cur = sizeof(unsigned) + sizeof(unsigned) + sizeof(int); // Mémoriser la postion où on doit mettre plus tard le nombre d'éléments (nb_elements)
	pos = lseek(fd_tmp, cur, SEEK_SET); // Positionne le curseur pour lire le nombre d'elements contenus dans la carte
	if (pos == -1){
		exit_with_error("lseek (tmp.map) !\n");
	}
	
	cur2 = cur + sizeof(int); // Déplacer le curseur d'un enregistrement
	pos2 = lseek(fd_tmp, cur2, SEEK_SET); // Se placer entre nb_elements et obj[i] dans le fichier
	if (pos2 == -1){
		exit_with_error("lseek (tmp.map) !\n");
	}

	for (int i = 0; i < old_nb_elements; ++i){
		// Lecture du numéro d'objet
		r = read(fd_tmp, &obj, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		// Lecture de l'abscisse de l'objet
		r = read(fd_tmp, &abs, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		// Lecture de l'ordonnée de l'objet
		r = read(fd_tmp, &ord, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}

		// Cas du rétrécissement: on recopie (sauvegarde) tous les éléménts de la carte compris entre (old_height-height) et height (cf. condition de if)
		if(height < old_height){
			if((ord >= (old_height-height))){
				nb_elements ++; // Incrémenter si un élément a pour ordonnée >= (old_height - height)
				ord -= (old_height-height); // Décalage des ordonnées de tous les éléments vers le haut (e.g le sol sera à l'ordonnée (old_height-1)-(old_height-height) = height-1)
				w = write(fd, &obj, sizeof(int));
				if(w == -1){
					exit_with_error("Writing error (%s)!\n", filename);
				}
				w = write(fd, &abs, sizeof(int));
				if(w == -1){
					exit_with_error("Writing error (%s)!\n", filename);
				}
				w = write(fd, &ord, sizeof(int));
				if(w == -1){
					exit_with_error("Writing error (%s)!\n", filename);
				}
			}
		}
		// Cas de l'agrandissement: on recopie tous les éléments de la carte, puis augmenter les deux mûrs
		if(height > old_height){ // Si condition pourra être supprimé ! 
			nb_elements ++; 
			ord += (height-old_height); // Décalage des ordonnées de tous les éléments vers le bas (e.g le sol sera à l'ordonnée (old_height-1)+(height-old_height) = height-1)
			w = write(fd, &obj, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			w = write(fd, &abs, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			w = write(fd, &ord, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
		} 
		
	}

	obj = 1; //Wall

	// Augmenter les mûrs lors de l'agrandissement de la hauteur
	if(height > old_height){
		for (int y = 0; y < height-old_height; ++y){
			nb_elements += 2; 
			// Mûr de gauche
			abs = 0; //Abscisse mûr côté gauche
			w = write(fd, &obj, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			w = write(fd, &abs, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			w = write(fd, &y, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			// Mûr de droite
			abs = width-1; //Abscisse mûr côté droite
			w = write(fd, &obj, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			w = write(fd, &abs, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
			w = write(fd, &y, sizeof(int));
			if(w == -1){
				exit_with_error("Writing error (%s)!\n", filename);
			}
		}
	}


	// Recopie des objects et leurs propriétés dans le nouveau fichier
	for (int i = 0; i < nb_objects; ++i){
		unsigned frames;
		int solidity;
		int generator;
		int collectible;
		int destructible;
		int length;

		r = read(fd_tmp, &frames, sizeof(unsigned));		
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		w = write(fd, &frames, sizeof(unsigned));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		r = read(fd_tmp, &solidity, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		w = write(fd, &solidity, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		r = read(fd_tmp, &generator, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		w = write(fd, &generator, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		r = read(fd_tmp, &collectible, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		w = write(fd, &collectible, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		r = read(fd_tmp, &destructible, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		w = write(fd, &destructible, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		r = read(fd_tmp, &length, sizeof(int));
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		w = write(fd, &length, sizeof(int)); // Ecriture de la longueur de chaque chemin dans le fichier (facilite la lecture des chaînes de caractères depuis le fichier)
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		char* path = (char*)malloc(length * sizeof(char)); // Allocation en mémoire d'un espace de stockage du chemin du fichier image
		r = read(fd_tmp, path, length);	
		if(r == -1){
			exit_with_error("Reading error (tmp.map) !\n");
		}
		path[length] = '\0'; // Marquege de fin de chaîne
		w = write(fd, path, strlen(path));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		path = NULL;
		free(path); // Libération de l'espace alloué
	}

	// Se positionner dans filename pour mettre à jour le nombre d'éléements qui valait 0 au départ
	pos = lseek(fd, cur, SEEK_SET);
	if (pos == -1){
		exit_with_error("lseek (%s) !\n", filename);
	}
	// Modification du nombre d'éléments de la carte
	w = write(fd, &nb_elements, sizeof(unsigned));
	if(w == -1){
		exit_with_error("Writing error (%s)!\n", filename);
	}

	// fermeture des 2 descripteurs de fichiers et suppression du fichier temporarire
	close(fd);
	close(fd_tmp);
	remove("tmp.map");
}

void setobjects(char* filename, int nb_objects, char const *properties[]){
	
	int fd, w, pos, cur, nb_elements, solidity, destructible, collectible, generator, length;
	unsigned frames; 

	// Ouverture en écriture du fichier 
	fd = open(filename, O_RDWR, 0666);
	if(fd == -1){
		exit_with_error("Opening error (%s)!\n", filename);
	}

	cur = sizeof(unsigned) + sizeof(unsigned); // Déplacer le curseur après les 4 premiers enregistrements
	pos = lseek(fd, cur, SEEK_SET); // Se positionner de cur octets dans le fichier, juste avants les objets (à réinitialiser)
	// Ecriture du nombre de types d'objets
	w = write(fd, &nb_objects, sizeof(int));
	if(w == -1){
		exit_with_error("Writing error !\n");
	}

	nb_elements = getelements(filename); 
	cur += sizeof(int) + sizeof(int); // Le curseur après nb_ojects et nb_elements
	cur += nb_elements * (3 * sizeof(int)); // Déplacer le curseur après les numéros des objets et leurs coordonnées
	pos = lseek(fd, cur, SEEK_SET); // Se positionner de cur octets dans le fichier, juste avants les objets (à réinitialiser)
	if (pos == -1){
		exit_with_error("lseek (%s) !\n", filename);
	}

	
	int j = 0;
	for (int i = 0; i < nb_objects; ++i){
		frames = (unsigned)atoi(properties[1+j]);
		solidity = atoi(convertir(properties[2+j]));
		generator = atoi(convertir(properties[5+j]));
		collectible = atoi(convertir(properties[4+j]));
		destructible = atoi(convertir(properties[3+j]));
		length = strlen(properties[0+j]);

		w = write(fd, &frames, sizeof(unsigned));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}

		w = write(fd, &solidity, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}
		w = write(fd, &generator, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}
		w = write(fd, &collectible, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}
		w = write(fd, &destructible, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}
		w = write(fd, &length, sizeof(int));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}
		w = write(fd, properties[0+j], strlen(properties[0+j]));
		if(w == -1){
			exit_with_error("Writing error (%s)!\n", filename);
		}
		j += 6; // 6 = nombre de propriétés pour chaquee objet (path, frames, ...)
	}
	ftruncate(fd, lseek(fd, 0, SEEK_CUR)); // Pour raccourcir le fichier
	
	close(fd);
}

int main(int argc, char const *argv[])
{
	int nb_arg = argc, nb_objects, old_nb_objects;

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
            {"getchangedate", no_argument, NULL, 'd'},
            {"getinfo", no_argument, NULL, 'i'},
            {"setwidth", required_argument, NULL, 'W'},
            {"setheight", required_argument, NULL, 'H'},
            {"setobjects", required_argument, NULL, 'O'},
            {NULL, 0, NULL, 0}
        };
        // getopt_long permet d'analyser les options (longues) de la ligne de commande
        val = getopt_long(argc, (char * const*)argv, optstring, long_options, &option_index);
        
        switch(val){ // Suivant les valeurs renvoyées par getopt_long
            case 'w':
				if(nb_arg != 3){
					exit_with_error("Three parameters !\nUsage: ./maputil <file> --option\n");
				}
                printf("La largeur de la carte est: %u\n", getwidth(filename));
                if (optarg) // S'il y a un paramètre de l'option getwidth indexée par 'w'
                    exit(EXIT_FAILURE);
                break;
            case 'h':
				if(nb_arg != 3){
					exit_with_error("Three parameters !\nUsage: ./maputil <file> --option\n");
				}
                printf("La hauteur de la carte est: %u\n", getheight(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'o':
				if(nb_arg != 3){
					exit_with_error("Three parameters !\nUsage: ./maputil <file> --option\n");
				}
                printf("Le nombre de types d'objets de la carte est: %d\n", getobjects(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'e':
				if(nb_arg != 3){
					exit_with_error("Three parameters !\nUsage: ./maputil <file> --option\n");
				}
                printf("Le nombre d'éléments (objets) de la carte est: %d\n", getelements(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'd':
				if(nb_arg != 3){
					exit_with_error("Three parameters !\nUsage: ./maputil <file> --option\n");
				}
            	printf("La date de dernière modification est: %s\n", getchangedate(filename));
				free(getchangedate(filename));
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'i':
				if(nb_arg != 3){
					exit_with_error("Three parameters !\nUsage: ./maputil <file> --option\n");
				}
            	getinfo(filename);
                if (optarg)
                    exit(EXIT_FAILURE);
                break;
            case 'W':
				if(nb_arg != 4){
					exit_with_error("Four parameters expected !\nUsage: ./maputil <file> --option <arg>\n");
				}
                
                if (optarg){ // S'il y a un paramètre de l'option getwidth indexée par 'w'
                	setwidth(filename, (unsigned)atoi(optarg));	
	                printf("La largeur de la carte est changée et est devenue %u\n", getwidth(filename));
	            }
                
                break;
            case 'H':
				if(nb_arg != 4){
					exit_with_error("Four parameters expected !\nUsage: ./maputil <file> --option <arg>\n");
				}
                
                if (optarg){
                	setheight(filename, (unsigned)atoi(optarg));	
                	printf("La hauteur de la carte est changée et est devenue %u\n", getheight(filename));
                }
                
                break;
            case 'O':
            	old_nb_objects = getobjects(filename); // Le nombre d'objet avant réinitialisation
            	nb_arg -= 3; // Le nombre d'arguments de l'option --setobjects (3 = ./maputil + filename + --setobjects)
            	nb_objects = nb_arg / 6; // 6 = le nombre de propriétés de chaue objet (path, frames, ...)
				if((nb_objects) < old_nb_objects){ // Si la nouvelle liste contient moins d'objets qu'à l'origine (6 = path + frames + ...)
					exit_with_error("The list must contain %d or more objects !\nUsage: %s <filename> --setobjects { <path> <frames> <solidity> <destructible> <collectible> <generator> }\n", old_nb_objects, argv[0]);
				}
                
                if (optarg){
                	setobjects(filename, nb_objects, argv+3);	
                	printf("Les objets de la carte sont réinitialisés avec succès !\n");
                }
                
                break;
            default: exit(EXIT_FAILURE);
            break;
        }
        /*
        if (optind < argc) {
            printf("Arguments ne constituant pas des options : ");
            while (optind < argc)
            printf("%s (optind = %d)", argv[optind++], (optind-1));
            printf("\n");
        }*/
    }

	return EXIT_SUCCESS;;
}
