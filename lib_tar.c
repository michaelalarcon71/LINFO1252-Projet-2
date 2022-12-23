#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib_tar.h"
#include <string.h>
#include <math.h>
//Funciona solo que con las carpetas se raya
/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd){
	int counter = 0;	
	char* buffer = malloc(512);
	lseek(tar_fd, 0, SEEK_SET);                 
	while (read(tar_fd, buffer, 512) != 0) {
		
		//Accedemos al bufferfer
		tar_header_t* arch = (tar_header_t*)buffer;
		//Sacamos el tamaño del archivo
		int tam = TAR_INT(arch->size);
		//printf("Tam: %d \n", tam);
		if(tam==0){break;}
		//Calculamos el salto -> falta lo del decimal
		int n_saltos = (tam/512)+1;
		//printf("N_saltos: %d \n", n_saltos);
		//Hacemos comprobaciones V

		//Test Version Value
		for (int i=0; i<2; i++) {
			if (arch->version[i] != '0') {
				return -2;
			}
		}
		//Test Magic Value
		char test1[6] = {'u','s','t','a','r','\0'};
		for (int i=0; i<6; i++) {
			if (arch->magic[i] != test1[i]) {	
		    		return -1;
			}
		}		
		//Test ChkSum Value
		int sumverif = 0;
	   	for (int i = 0; i < 512; i++) {
			if (i >= 148 && i < 156) {
				sumverif += 32; // ASCII 32 = space
			}
			else sumverif += (int) *((char*)arch+i);
		}
		if (TAR_INT(arch->chksum) != sumverif) {
	    		return -3;
	    	}
		// Volvemos al principio
		lseek(tar_fd,n_saltos*512, SEEK_CUR);
		counter ++;
    }

    	free(buffer);

	return counter;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
	if (check_archive(tar_fd) < 0) {
		printf("Check Archive Failed\n");
		return 0;
	} 
	

	lseek(tar_fd, 0, SEEK_SET); //We will reset the head pointer every time we call check_archive
	char* buffer = malloc(512);
	while (read(tar_fd, buffer, 512) != 0){
		if (strcmp(path, buffer) == 0) {
			return 1;
		}
	}
	free(buffer);
	return 0;
}


/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
	if (check_archive(tar_fd) < 0) {
		printf("Check Archive Failed\n");
		return 0;
	}
	
	lseek(tar_fd, 0, SEEK_SET); 
	char* buffer = malloc(512);
	while (read(tar_fd, buffer, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buffer;
		if (strcmp(path, buffer) == 0) {
			if (arch->typeflag == '5'){
				return 1;
			}
		}
	}
	free(buffer);
	return 0;   
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
	if (check_archive(tar_fd) < 0) {
		printf("Check Archive Failed\n");
		return 0;
	} 

	lseek(tar_fd, 0, SEEK_SET); 
	char* buffer = malloc(512);
	while (read(tar_fd, buffer, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buffer;
		if (strcmp(path, buffer) == 0) {
			if (arch->typeflag == '0'){
				return 1;
			}
		}
	}
	free(buffer);
	return 0;   
}
 
 //no entiendo bien lo que es un link
/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    if (check_archive(tar_fd) < 0) {
		printf("Check Archive Failed\n");
		return 0;
	}
	
	lseek(tar_fd, 0, SEEK_SET);
	char* buffer = malloc(512);
	while (read(tar_fd, buffer, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buffer;
		if (strcmp(path, buffer) == 0) {
			printf("%s\n", arch->linkname);
			if (strcmp(arch->linkname,""))
				return 1;
			else
				return 0;
		}
	}
	free(buffer);
	return 0; 
}


/**
 * Lists the entries at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entry in entries.
 *                   The callee set it to the number of entry listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    return 0;
}


/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination bufferfer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination bufferfer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
	if (check_archive(tar_fd) < 0) {
		printf("Check Archive Failed\n");
		return 0;
	}
    
	int data;
	int res;

	char buffer[512];
	char* t;
	char var[32];
	char new[32];

	strcpy(var,path);
	lseek(tar_fd,0,SEEK_SET);

	while (read(tar_fd, buffer, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buffer;
		int fsize = strtol(arch->size,NULL,8);

		if (strcmp(var, buffer) == 0) {
			if(arch->typeflag == "0"){
				if (offset < 0 || offset > fsize){
					*len = 0;
					return -2;
				}
				if (*len + offset > fsize){
					*len = fsize - offset;
				}

				t = malloc(*len);
				data = read(tar_fd,t,offset);
				free(t);

				t = malloc(*len);
				data = read(tar_fd,t,*len);
				memcpy(dest,t,data);
				free(t);

				res = fsize - data - offset;
				*len = data;
				return res;


			}
		}
		if( strcpy(arch->linkname, "")){
			for(int i = sizeof(var) - 1; var[i] != '/' && i >= 0; i--){
				strcpy(new,var);
				strcat(new, arch->linkname);
				strcpy(var,new);

				lseek(tar_fd,0,SEEK_SET);
				continue;
			}
		}
		*len = 0;
		return -1;
	}
	*len = 0;
	return -1;
}

