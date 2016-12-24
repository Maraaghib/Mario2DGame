#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"

#ifdef PADAWAN

/*Cette fonction gère les erreurs, d'ouverture, de lecture et d'criture des fichiers*/
void catch (char* message){
	perror(message);
	exit(EXIT_FAILURE);
}

/*Cette fonction crée une nouvelle carte*/
void map_new (unsigned width, unsigned height)
{
	map_allocate (width, height); //alloue  une  carte  en  mémoire de dimension width * height

	for (int x = 0; x < width; x++)
	map_set (x, height - 1, 0); // Ground

	for (int y = 0; y < height - 1; y++) {
	map_set (0, y, 1); // Wall
	map_set (width - 1, y, 1); // Wall
	}

	map_object_begin (6); // Le nombre types d'objets

	// Texture pour le sol
	map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
	// Mur
	map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
	// Gazon
	map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
	// Marbre
	map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
	// Fleur
	map_object_add ("images/flower.png", 1, MAP_OBJECT_AIR);
	// Pièces
	map_object_add ("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE);

	map_object_end ();

}

/* Fonction de sauvegarde de la carte*/
void map_save (char *filename)
{
	int fd, w, nb_objects, obj, abs, ord, length, nb_elements = 0; // nb_elemnets = nombre d'objets non-vides
	unsigned frames, width, height; // Le nombre de sprites
	int solidity; // Enumération pour la solidité de l'objet
	int destructible;
	int collectible;
	int generator;
	width = map_width();
	height = map_height();
	nb_objects = map_objects();
	
	// Ouverture en écriture du fichier où doit être stockée la carte
	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC);
	if(fd == -1){
		catch("Opening error !\n");
	}
	// Ecriture de la largeur de la carte
	w = write(fd, &width, sizeof(unsigned));
	if(w == -1){
		catch("Writing error !\n");
	}
	// Ecriture de la largeur de la carte
	w = write(fd, &height, sizeof(unsigned));
	if(w == -1){
		catch("Writing error !\n");
	}
	// Ecriture du nombre de types d'objets
	w = write(fd, &nb_objects, sizeof(int));
	if(w == -1){
		catch("Writing error !\n");
	}
	// Comptage des nombres d'objets non-vides (utile pour la lecture afin de faire une boucle uniquement sur les objets non-vides)
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			obj = map_get(x, y);
			
			if(obj != MAP_OBJECT_NONE){
				nb_elements ++;
			}
			
		}
	}
	// Ecriture de nombre d'éléments non-vides
	w = write(fd, &nb_elements, sizeof(int)); // Ecriture du nombre d'objets dans la carte
	if(w == -1){
		catch("Writing error !\n");
	}
	// Ecriture des objets on-vides et leurs coordoonnées respectives
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			obj = map_get(x, y);
			
			if(obj != MAP_OBJECT_NONE){
				abs = x;
				ord = y;
				w = write(fd, &obj, sizeof(int));
				if(w == -1){
					catch("Writing error !\n");
				}
				w = write(fd, &abs, sizeof(int));
				if(w == -1){
					catch("Writing error !\n");
				}
				w = write(fd, &ord, sizeof(int));
				if(w == -1){
					catch("Writing error !\n");
				}
			}
			
		}
	}
	// Ecriture des types d'objets et leurs caractéristiques correspondantes
	for (int i = 0; i < nb_objects; ++i){
		frames = map_get_frames(i);
		solidity = map_get_solidity(i);
		generator = map_is_generator(i);
		collectible = map_is_collectible(i);
		destructible = map_is_destructible(i);
		length = strlen(map_get_name(i));

		w = write(fd, &frames, sizeof(unsigned));
		if(w == -1){
			catch("Writing error !\n");
		}
			w = write(fd, &solidity, sizeof(int));
		if(w == -1){
			catch("Writing error !\n");
		}
		w = write(fd, &generator, sizeof(int));
		if(w == -1){
			catch("Writing error !\n");
		}
		w = write(fd, &collectible, sizeof(int));
		if(w == -1){
			catch("Writing error !\n");
		}
		w = write(fd, &destructible, sizeof(int));
		if(w == -1){
			catch("Writing error !\n");
		}
		w = write(fd, &length, sizeof(int)); // Ecriture de la longueur de chaque chemin dans le fichier (facilite la lecture des chaînes de caractères depuis le fichier)
		if(w == -1){
			catch("Writing error !\n");
		}
		w = write(fd, map_get_name(i), strlen(map_get_name(i)));
		if(w == -1){
			catch("Writing error !\n");
		}
	}
	
	close(fd);
	printf("Carte sauvegardée avec succès !\n");
	
}

/* Fonction de chargement de la carte*/
void map_load (char *filename)
{
	// TODO
	exit_with_error ("Map load is not yet implemented\n");
}

#endif
