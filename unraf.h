int main(int argc, char* argv[]);

#define HEADER_FIELDS   5
#define FIELD_SIZE      4
#define FIELD_SIZE_BITS 32          // = 4 * 8
#define HEADER_ID       0x18be0ef0
#define HEADER_VERSION  1
#define FILE_LIST       3
#define PATH_LIST       4
#define FILE_FIELD_SIZE 16
#define PATH_FIELD_SIZE 8

struct lookup_table {
	unsigned int offset;
	unsigned int length;
	char* filename;
};

struct file_list {
	unsigned int hash:   FIELD_SIZE_BITS;
	unsigned int offset: FIELD_SIZE_BITS;
	unsigned int length: FIELD_SIZE_BITS;
	unsigned int index:  FIELD_SIZE_BITS;
};

static int process(char* meta_file, char* data_file, char* out,
							bool auto_decompress);
static bool read_lookup_table(char* meta_file, struct lookup_table** table,
						unsigned int* list_size);
static bool read_header(FILE* meta, unsigned int header[]);
static bool read_size(FILE* meta, unsigned int offset, unsigned int* size);
static bool read_file_list(FILE* meta, struct file_list* list_data,
						unsigned int list_size);

struct path_list {
	unsigned int offset: FIELD_SIZE_BITS;
	unsigned int length: FIELD_SIZE_BITS;
};

static bool read_path_list(FILE* meta, char** filenames, unsigned int size,
							unsigned int offset);
static void clear_filenames(char** filenames, unsigned int length);
static void generate_lookup_table(struct lookup_table** table,
		unsigned int list_size, struct file_list* meta, char** names);
static bool extract(char* file, char* out, struct lookup_table* table,
				unsigned int length, bool auto_decompress);
static bool extract_entry(FILE* stream, char* out, struct lookup_table entry,
							bool auto_decompress);
static bool mkpath(char* file_path, mode_t mode);
static int inflate_part(FILE *source, FILE *dest, unsigned int length,
							void* in, void* out);
