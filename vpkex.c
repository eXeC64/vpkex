#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>

void print_usage(const char* app)
{
  fprintf(stderr, "Usage: %s <archive> [file]\n", app);
  fprintf(stderr, "\nIf file is ommitted, a file list is printed.\n"
                  "Otherwise the contents of the file is output to stdout.\n");
}

int read_u16(FILE* fp, uint16_t* out)
{
  int len = fread(out, 2, 1, fp);
  *out = le16toh(*out);
  return len == 1;
}

int read_u32(FILE* fp, uint32_t* out)
{
  int len = fread(out, 4, 1, fp);
  *out = le32toh(*out);
  return len == 1;
}

int read_str(char* buf, size_t len, FILE* fp)
{
  size_t pos;
  for(pos = 0; pos < len - 1; ++pos)
  {
    buf[pos] = fgetc(fp);
    if(buf[pos] == EOF) {
      buf[pos] = 0;
    }
    if(buf[pos] == 0 || buf[pos] == '\n') {
      buf[pos+1] = 0;
      break;
    }
  }
  return pos+1;
}

int main(int argc, char** argv)
{
  if(argc < 2) {
    print_usage(argv[0]);
    exit(1);
  }

  const char* apath = argv[1];
  const char* fpath = argc > 2 ? argv[2] : 0;

  FILE* af = fopen(apath, "r");
  if(!af) {
    fprintf(stderr, "Failed to open archive: %s\n", apath);
    exit(1);
  }

  uint32_t magic;
  read_u32(af, &magic);

  if(magic != 0x55aa1234) {
    fprintf(stderr, "Invalid vpk file: %s\n", apath);
    fclose(af);
    exit(1);
  }

  uint32_t version;
  read_u32(af, &version);
  if(version != 1 && version != 2) {
    fprintf(stderr, "Unsupported vpk version: %i\n", version);
    fclose(af);
    exit(1);
  }

  uint32_t tree_size;
  read_u32(af, &tree_size);

  //Move to directory
  if(version == 2) {
    fseek(af, 16, SEEK_CUR);
  }

  uint16_t pre_bytes = 0;
  uint16_t archive_index = 0;
  uint32_t archive_offset = 0;
  uint32_t archive_length = 0;

  int found = 0;
  while(!found) {
    char ext[32];
    read_str(ext, sizeof(ext), af);
    if(ext[0] == 0) {
      break;
    }
    while(!found) {
      char path[256];
      read_str(path, sizeof(path), af);
      if(path[0] == 0) {
        break;
      }
      while(!found) {
        char file[128];
        read_str(file, sizeof(file), af);
        if(file[0] == 0) {
          break;
        }
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s.%s", path, file, ext);

        //If we have fpath compare, otherwise print path
        if(fpath) {
          //Get needed information and extract data
          if(strcmp(fpath, fullpath) == 0) {
            fseek(af, 4, SEEK_CUR); //skip CRC
            read_u16(af, &pre_bytes);
            read_u16(af, &archive_index);
            read_u32(af, &archive_offset);
            read_u32(af, &archive_length);
            fseek(af, 2, SEEK_CUR); //skip the terminator
            found = 1;
          } else {
            //Skip file information section
            fseek(af, 18, SEEK_CUR);
          }
        } else {
          fprintf(stdout, "%s\n", fullpath);
          //Skip file information section
          fseek(af, 18, SEEK_CUR);
        }
      }
    }
  }


  if(fpath && !found) {
    fclose(af);
    fprintf(stderr, "Could not find '%s' in archive.\n", fpath);
    exit(1);
  } else if(fpath && found) {
    int dir_len = strlen(apath) - 7;
    char abpath[512];
    char aipath[512];
    memset(abpath, 0, sizeof(abpath));
    strncpy(abpath, apath, dir_len);
    snprintf(aipath, sizeof(aipath), "%s%.3i.vpk", abpath, archive_index);
    FILE* aif = fopen(aipath, "r");
    if(!aif) {
      fclose(af);
      fprintf(stderr, "Failed to open archive file: %s\n", aipath);
      exit(1);
    }
    fseek(aif, archive_offset, SEEK_SET);
    char buf[1024];
    size_t sent = 0;
    while(sent < archive_length) {
      int chunk = archive_length - sent < 1024 ? archive_length - sent: 1024;
      fread(buf, 1, chunk, aif);
      fwrite(buf, 1, chunk, stdout);
      sent += chunk;
    }
    fclose(aif);
    fclose(af);
    exit(0);
  } else {
    fclose(af);
  }

  return 0;
}
