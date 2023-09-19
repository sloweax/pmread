#include "util.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

bool startswith(const char *str, const char *pre) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

void print_map_region(FILE *f, long unsigned start, long unsigned end,
                      const char *region) {
  if (fseek(f, start, SEEK_SET) != 0) {
    fprintf(stderr, "could not read region '%s'\n", region);
    return;
  }

  char *buf = emalloc(end - start);

  if (fread(buf, 1, end - start, f) != end - start) {
    free(buf);
    fprintf(stderr, "could not read region '%s'\n", region);
    return;
  }

  efwrite(buf, 1, end - start, stdout);

  free(buf);
}

int main(int argc, char **argv) {
  long unsigned start, end;
  FILE *fmem, *fmaps;
  char pathmem[PATH_MAX], pathmaps[PATH_MAX];

  if (argc == 1)
    die("usage: %s PID [REGIONS...]\n"
        "read map REGIONS of PID and writes to stdout\n"
        "examples:\n"
        "\t%s PID <addr start>-<addr end>\n"
        "\t%s PID all (read all REGIONS)\n"
        "\t%s PID path:[heap] path:[stack] path:/path/to/file (read REGION by "
        "path)\n"
        "\t%s PID inode:0 (read REGION by inodeid)",
        argv[0], argv[0], argv[0], argv[0], argv[0]);

  if (argc <= 2)
    die("usage: %s pid [REGIONS...]", argv[0]);

  snprintf(pathmem, PATH_MAX, "/proc/%s/mem", argv[1]);
  snprintf(pathmaps, PATH_MAX, "/proc/%s/maps", argv[1]);

  fmem = efopen(pathmem, "rb");
  fmaps = efopen(pathmaps, "rb");

  size_t mapslen = 0;
  size_t mapslines = 0;
  int c;
  while ((c = fgetc(fmaps)) != EOF) {
    if (c == '\n')
      mapslines++;
    mapslen++;
  }

  efseek(fmaps, 0, SEEK_SET);
  char mapsdata[mapslen + 1];
  efread(mapsdata, 1, mapslen, fmaps);
  mapsdata[mapslen] = '\0';

  int mapc = 0;
  char *mapv[mapslines];
  char *token = strtok(mapsdata, "\n");
  mapv[mapc++] = token;
  while (1) {
    token = strtok(NULL, "\n");
    if (token == NULL)
      break;
    mapv[mapc++] = token;
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "all") == 0) {
      argc = 3;
      argv[2] = "all";
      break;
    }
  }

  for (int i = 2; i < argc; i++) {
    if (sscanf(argv[i], "%lx-%lx", &start, &end) == 2) {
      // <addr start>-<addr end> regions
      char tmp[500];
      snprintf(tmp, 500, "%lx-%lx", start, end);
      if (start > end) {
        fprintf(stderr, "invalid region '%s'\n", tmp);
        continue;
      }
      print_map_region(fmem, start, end, tmp);
    } else {
      for (int j = 0; j < mapc; j++) {
        char mode[32], path[PATH_MAX + 1];
        long unsigned offset, major, minor;
        int inode;
        path[0] = '\0';

        int n =
            sscanf(mapv[j], "%lx-%lx %s %lx %lx:%lx %d %" STR(PATH_MAX) "[^\n]",
                   &start, &end, mode, &offset, &major, &minor, &inode, path);

        if (n < 7)
          die("could not parse '%s'", pathmaps);

        if (strcmp(argv[i], "all") == 0)
          print_map_region(fmem, start, end, mapv[j]);

        else if (startswith(argv[i], "inode:")) {
          int inode2;

          if (sscanf(argv[i], "inode:%d", &inode2) != 1) {
            fprintf(stderr, "invalid inode '%s'\n", argv[i]);
            break;
          }

          if (inode == inode2)
            print_map_region(fmem, start, end, mapv[j]);
        }

        else if (startswith(argv[i], "path:")) {
          if (n != 8)
            continue;

          char path2[PATH_MAX + 1];

          if (sscanf(argv[i], "path:%" STR(PATH_MAX) "[^\n]", path2) != 1) {
            fprintf(stderr, "invalid path '%s'\n", argv[i]);
            break;
          }

          if (strcmp(path, path2) == 0)
            print_map_region(fmem, start, end, mapv[j]);
        }

        else {
          fprintf(stderr, "invalid region '%s'\n", argv[i]);
          break;
        }
      }
    }
  }

  fclose(fmem);
  fclose(fmaps);

  return 0;
}
