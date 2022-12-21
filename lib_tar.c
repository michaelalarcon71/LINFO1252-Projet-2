#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib_tar.h"
#include <string.h>
#include <math.h>

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
int check_archive(int tar_fd) {
	int count = 0;
	char* buf = malloc(512);
	lseek(tar_fd, 0, SEEK_SET);
	while (read(tar_fd, buf, 512) != 0) {
		tar_header_t* arch = (tar_header_t*) buf;
		if (arch->typeflag == '0' || arch->typeflag == '5' || arch->typeflag == '2') {
			count++;
			//Test Magic Value
			char test1[6] = {'u','s','t','a','r','\0'};
			for (int i=0; i<6; i++) {
				if (arch->magic[i] != test1[i]) {	
			    		return -1;
				}
			}
			
			//Test Version Value
			for (int i=0; i<2; i++) {
				if (arch->version[i] != '0') {
					return -2;
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
    		}
    	}
    	lseek(tar_fd, 0, SEEK_SET);
    	free(buf);
    	if (count == 0) {
    		return -1;
    	}
	return count;
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
		printf("CheckArchive Failed\n");
		return 0;
	} 
	
	lseek(tar_fd, 0, SEEK_SET); //Reset la tete de lecture
	
	char* buf = malloc(512);
	while (read(tar_fd, buf, 512) != 0){
		if (strcmp(path, buf) == 0) {
			return 1;
		}
	}
	free(buf);
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
		printf("CheckArchive Failed\n");
		return 0;
	}
	
	lseek(tar_fd, 0, SEEK_SET); //Reset la tete de lecture

	char* buf = malloc(512);
	while (read(tar_fd, buf, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buf;
		if (strcmp(path, buf) == 0) {
			if (arch->typeflag == '5'){
				return 1;
			}
		}
	}
	free(buf);
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
		printf("CheckArchive Failed\n");
		return 0;
	} 

	lseek(tar_fd, 0, SEEK_SET); //Reset la tete de lecture

	char* buf = malloc(512);
	while (read(tar_fd, buf, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buf;
		if (strcmp(path, buf) == 0) {
			if (arch->typeflag == '0'){
				return 1;
			}
		}
	}
	free(buf);
	return 0;   
}

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
		printf("CheckArchive Failed\n");
		return 0;
	}
	
	lseek(tar_fd, 0, SEEK_SET); //Reset la tete de lecture

	char* buf = malloc(512);
	while (read(tar_fd, buf, 512) != 0){
		tar_header_t* arch = (tar_header_t*) buf;
		if (strcmp(path, buf) == 0) {
			printf("%s\n", arch->linkname);
			if (strcmp(arch->linkname,""))
				return 1;
			else
				return 0;
		}
	}
	free(buf);
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
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {

    return 0;
}

