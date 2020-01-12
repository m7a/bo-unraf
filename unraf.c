/*
 * Unraf 1.0.0.0: A simple tool for extracting pairs of ".raf" and ".raf.dat"
 * 									files
 *
 * Copyright (C) 2013  Ma_Sys.ma
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

// For decompression functionality
#include <assert.h>
#include <zlib.h>

// Defines structures, constants and functions
#include "unraf.h"

int main(int argc, char* argv[]) {
	if(argc < 3) {
		puts("unraf 1.0.0.0, Copyright (c) 2013 Ma_Sys.ma.");
		puts("For further info send an e-mail to Ma_Sys.ma@web.de.\n");
		printf("Usage: %s <file.raf> <file.raf.dat> [dir] [-r]\n",
								argv[0]);
		puts("For [dir] a leading slash is not optional!");
		puts("-r is for \"raw\" and does not decompress files.");
		puts("Use -r if problems occur while processing the .dat.");
		puts("If -r is given, dir needs to be given as well.");
		return EXIT_FAILURE;
	}

	bool auto_decompress = true;
	char* odir;
	if(argc == 3) {
		odir = "./";
	} else {
		odir = argv[3];
		if(argc == 5 && strcmp(argv[4], "-r") == 0) {
			auto_decompress = false;
		}
	}

	return process(argv[1], argv[2], odir, auto_decompress);
}

// The specifications for the file format can be found on either of these:
//  * http://leagueoflegends.wikia.com/wiki/RAF:_Riot_Archive_File
//  * http://na.leagueoflegends.com/board/showthread.php?t=649185&page=8 
//  * https://github.com/mriedel/winraf/raw/master/Riot%20archive%20file%20
//  								format.docx
// I have given all links I know, because one can never be sure how long these
// links will be available. Therefore I think redundancy might be helpful.
static int process(char* meta_file, char* data_file, char* out,
							bool auto_decompress) {
	struct lookup_table** table = malloc(sizeof(struct lookup_table*));
	unsigned int length;
	if(!read_lookup_table(meta_file, table, &length)) {
		free(table);
		return EXIT_FAILURE;
	}
	if(!extract(data_file, out, *table, length, auto_decompress)) {
		free(*table);
		free(table);
		return EXIT_FAILURE;
	}
	free(*table);
	free(table);
	return EXIT_SUCCESS;
}

static bool read_lookup_table(char* meta_file, struct lookup_table** table,
						unsigned int* list_size) {
	FILE* meta = fopen(meta_file, "rb");
	if(meta == NULL) {
		printf("Unable to open \"%s\".\n", meta_file);
		return false;
	}

	unsigned int header[HEADER_FIELDS];
	if(!read_header(meta, header) ||
			!read_size(meta, header[FILE_LIST], list_size)) {
		return false;
	}

	struct file_list* list_data =
				malloc(sizeof(struct file_list) * *list_size);
	unsigned int path_list_size;
	if(!read_file_list(meta, list_data, *list_size) ||
			//           skip first field (tablesize not important)
			!read_size(meta, header[PATH_LIST] + FIELD_SIZE,
							&path_list_size)) {
		free(list_data);
		return false;
	}

	if(*list_size != path_list_size) {
		fputs("Path list and file list length differ.", stderr);
		fclose(meta);
		free(list_data);
		return false;
	}

	char** filenames = malloc(sizeof(char*) * *list_size);
	if(!read_path_list(meta, filenames, *list_size, header[PATH_LIST])) {
		free(list_data);
		clear_filenames(filenames, *list_size);
		return false;
	}

	generate_lookup_table(table, *list_size, list_data, filenames);

	free(list_data);
	fclose(meta);
	return true;
}

static bool read_header(FILE* meta, unsigned int header[]) {
	if(fread(header, FIELD_SIZE, HEADER_FIELDS, meta) != HEADER_FIELDS) {
		fputs("Meta file header incomplete.", stderr);
		fclose(meta);
		return false;
	}
	if(header[0] != HEADER_ID) {
		fprintf(stderr, "Found header 0x%x instead of 0x%x.\n",
							header[0], HEADER_ID);
		fputs("The file might not be a valid .raf file.", stderr);
		fclose(meta);
		return false;
	}
	if(header[1] != HEADER_VERSION) {
		fprintf(stderr,
			"Warning: Found version 0x%x instead of 0x%x.\n",
						header[1], HEADER_VERSION);
		fputs("Expect segfault and decoding failure.", stderr);
	}
	return true;
}

static bool read_size(FILE* meta, unsigned int offset, unsigned int* size) {
	if(fseek(meta, offset, SEEK_SET) != 0) {
		fprintf(stderr, "Unable to seek to 0x%x\n", offset);
		fclose(meta);
		return false;
	}
	if(fread(size, FIELD_SIZE, 1, meta) != 1) {
		fputs("Could not read list size.", stderr);
		fclose(meta);
		return false;
	}
	return true;
}

static bool read_file_list(FILE* meta, struct file_list* list_data,
						unsigned int list_size) {
	if(fread(list_data, FILE_FIELD_SIZE, list_size, meta) != list_size) {
		fputs("Could not read whole file list.", stderr);
		fclose(meta);
		return false;
	}
	return true;
}

static bool read_path_list(FILE* meta, char** filenames, unsigned int size,
							unsigned int offset) {
	struct path_list* path_list = malloc(sizeof(struct path_list) * size);
	if(fread(path_list, PATH_FIELD_SIZE, size, meta) != size) {
		fputs("Could not read whole path list.", stderr);
		fclose(meta);
		free(path_list);
		return false;
	}
	size_t i;
	for(i = 0; i < size; i++) {
		if(fseek(meta, offset + path_list[i].offset, SEEK_SET) != 0) {
			fprintf(stderr, "Unable to seek string %td.\n", i);
			fclose(meta);
			free(path_list);
			return false;
		}
		filenames[i] = malloc(sizeof(char) * path_list[i].length);
		if(fread(filenames[i], sizeof(char), path_list[i].length, meta)
						!= path_list[i].length) {
			fputs("Could not read complete filename.", stderr);
			fclose(meta);
			free(path_list);
			return false;
		}
	}
	free(path_list);
	return true;
}

static void clear_filenames(char** filenames, unsigned int length) {
	size_t i;
	for(i = 0; i < length; i++) {
		if(filenames[i] != NULL) {
			free(filenames[i]);
		}
	}
	free(filenames);
}

static void generate_lookup_table(struct lookup_table** table,
		unsigned int list_size, struct file_list* meta, char** names) {
	*table = malloc(sizeof(struct lookup_table) * list_size);
	size_t i, j;
	for(i = 0; i < list_size; i++) {
		(*table)[i].offset   = meta[i].offset;
		(*table)[i].length   = meta[i].length;
		(*table)[i].filename = names[meta[i].index];
	}
	// Insertionsort
	// Source:
	// 	"Algorithmen in Java", Prof. Dr. Hans Werner Lang,
	// 	Oldenbourg Verlag, 2012, p. 7
	for(i = 1; i < list_size; i++) {
		j = i;
		struct lookup_table compare = (*table)[j];
		while(j > 0 && (*table)[j - 1].offset > compare.offset) {
			(*table)[j] = (*table)[j - 1];
			j--;
		}
		(*table)[j] = compare;
	}
	free(names);
}

static bool extract(char* file, char* out, struct lookup_table* table,
				unsigned int size, bool auto_decompress) {
	FILE* stream = fopen(file, "rb");
	if(stream == NULL) {
		fprintf(stderr, "Unable to open \"%s\".\n", file);
		return false;
	}
	size_t i;
	for(i = 0; i < size; i++) {
		if(!extract_entry(stream, out, table[i], auto_decompress)) {
			fclose(stream);
			return false;
		}
	}
	fclose(stream);
	return true;
}

static bool extract_entry(FILE* stream, char* out, struct lookup_table entry,
							bool auto_decompress) {
	char path[4096];
	strcpy(path, out);
	strcat(path, entry.filename);

	if(!mkpath(path, 0755)) {
		fprintf(stderr, "Unable to mkdir -p %s\n", path);
		return false;
	}

	if(fseek(stream, entry.offset, SEEK_SET) != 0) {
		fprintf(stderr, "Unable to seek to %x.\n", entry.offset);
		return false;
	}
	void* data = malloc(entry.length);
	if(!auto_decompress) {
		if(fread(data, 1, entry.length, stream) != entry.length) {
			fprintf(stderr, "Data for \"%s\" are incomplete.\n",
							entry.filename);
			free(data);
			return false;
		}
	}

	FILE* ostream = fopen(path, "wb");
	if(ostream == NULL) {
		fprintf(stderr, "Unable to open \"%s\" for writing.\n", path);
		free(data);
		return false;
	}
	bool decode_failure = false;
	if(auto_decompress) {
		void* obuf = malloc(entry.length);
		// data is initialized inside the function...
		if(inflate_part(stream, ostream, entry.length, data, obuf)
								!= Z_OK) {
			fprintf(stderr,
				"Warning: Zlib decoding failed for \"%s\"\n",
							entry.filename);
			// Prepare for raw decoding...
			if(fseek(ostream, 0, SEEK_SET) != 0) {
				fputs("Could not reset file pointer.", stderr);
				free(obuf);
				free(data);
				fclose(ostream);
				return false;
			}
			decode_failure = true;
		}
		free(obuf);
	}
	if(!auto_decompress || decode_failure) {
		if(fwrite(data, 1, entry.length, ostream) != entry.length) {
			fprintf(stderr, "Wrote incomplete data to \"%s\"\n",
									path);
			fclose(ostream);
			free(data);
			return false;
		}
	}
	fclose(ostream);

	free(data);
	return true;
}

// Copied and slightly adapted from http://stackoverflow.com/questions/2336242/
// 					recursive-mkdir-system-call-on-unix
// Accessed 06.01.2013
static bool mkpath(char* file_path, mode_t mode) {
	char* p;
	for(p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';
		if(mkdir(file_path, mode) == -1) {
			if(errno != EEXIST) {
				*p = '/';
				// Original implementation used integer return
				// values.
				return false;
			}
		}
		*p='/';
	}
	return true;
}

// This has been copied directly from the zlib example "zpipe.c"
// compare /usr/share/doc/zlib1g-dev/examples after having installed zlib1g-dev
// on Debian or examples/zpipe.c from the zlib sourcecode.
// To match the rest of the program, the indentation has been changed and a
// loop that was not needed with this implementation has been removed.
static int inflate_part(FILE *source, FILE *dest, unsigned int length,
							void* in, void* out) {
	int ret;
	unsigned have;
	z_stream strm;

	// allocate inflate state
	strm.zalloc   = Z_NULL;
	strm.zfree    = Z_NULL;
	strm.opaque   = Z_NULL;
	strm.avail_in = 0;
	strm.next_in  = Z_NULL;
	ret           = inflateInit(&strm);
	if(ret != Z_OK) {
		return ret;
	}

	strm.avail_in = fread(in, 1, length, source);
	if(ferror(source)) {
		(void)inflateEnd(&strm);
		return Z_ERRNO;
	}
	strm.next_in = in;

	// run inflate() on input until output buffer not full
	do {
		strm.avail_out = length;
		strm.next_out = out;
		ret = inflate(&strm, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR); // state not clobbered
		switch(ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR; // and fall through
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
		}
		have = length - strm.avail_out;
		if(fwrite(out, 1, have, dest) != have || ferror(dest)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
	} while(strm.avail_out == 0);

	// clean up and return
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
