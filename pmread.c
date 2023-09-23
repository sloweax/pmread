#include "util.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAPINFOPATHF "%" STR(PATH_MAX) "[^\n]"

typedef struct {
  char mode[5], path[PATH_MAX];
  long unsigned start, end, offset, major, minor;
  int inode;
} Mapinfo;

bool startswith(const char *str, const char *pre);
void parse_map_line(Mapinfo *m, const char *line);
void print_map_region(FILE *f, const Mapinfo *m);
void usage(int argc, char **argv, int status);
void popv(int *argc, char **argv, int index);

int main(int argc, char **argv) {
  FILE *fmem, *fmaps;
  char pathmem[PATH_MAX], pathmaps[PATH_MAX];

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
      usage(argc, argv, 0);
    else if (startswith(argv[i], "-")) {
      die("unknown option %s", argv[i]);
    }
  }

  if (argc == 1)
    die("missing PID\n%s -h for help", argv[0]);

  if (argc == 2)
    die("missing REGION\n%s -h for help", argv[0]);

  snprintf(pathmem, PATH_MAX, "/proc/%s/mem", argv[1]);
  snprintf(pathmaps, PATH_MAX, "/proc/%s/maps", argv[1]);

  if (!(access(pathmem, F_OK) == 0))
    die("invalid PID %s", argv[1]);

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
  while ((token = strtok(NULL, "\n")))
    mapv[mapc++] = token;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "all") == 0) {
      argc = 3;
      argv[2] = "all";
      break;
    } else if (strcmp(argv[i], "list") == 0) {
      for (int j = 0; j < mapc; j++)
        puts(mapv[j]);
      return 0;
    }
  }

  for (int i = 2; i < argc; i++) {
    Mapinfo m;

    if (sscanf(argv[i], "%lx-%lx", &m.start, &m.end) == 2) {
      // <addr start>-<addr end> regions
      if (m.start > m.end) {
        fprintf(stderr, "invalid region %lx-%lx\n", m.start, m.end);
        continue;
      }

      m.path[0] = '\0';
      print_map_region(fmem, &m);
    } else {
      for (int j = 0; j < mapc; j++) {
        parse_map_line(&m, mapv[j]);

        if (strcmp(argv[i], "all") == 0)
          print_map_region(fmem, &m);

        else if (startswith(argv[i], "inode:")) {
          int inode;

          if (sscanf(argv[i], "inode:%d", &inode) != 1) {
            fprintf(stderr, "invalid inode '%s'\n", argv[i]);
            break;
          }

          if (m.inode == inode)
            print_map_region(fmem, &m);
        }

        else if (startswith(argv[i], "dev:")) {
          long unsigned major, minor;

          if (sscanf(argv[i], "dev:%lx:%lx", &major, &minor) != 2) {
            fprintf(stderr, "invalid dev '%s'\n", argv[i]);
            break;
          }

          if (m.major == major && m.minor == minor)
            print_map_region(fmem, &m);
        }

        else if (startswith(argv[i], "major:")) {
          long unsigned major;

          if (sscanf(argv[i], "major:%lx", &major) != 1) {
            fprintf(stderr, "invalid major '%s'\n", argv[i]);
            break;
          }

          if (m.major == major)
            print_map_region(fmem, &m);
        }

        else if (startswith(argv[i], "minor:")) {
          long unsigned minor;

          if (sscanf(argv[i], "minor:%lx", &minor) != 1) {
            fprintf(stderr, "invalid minor '%s'\n", argv[i]);
            break;
          }

          if (m.minor == minor)
            print_map_region(fmem, &m);
        }

        else if (startswith(argv[i], "path:")) {
          if (m.path[0] == '\0')
            continue;

          char path[PATH_MAX];

          if (sscanf(argv[i], "path:" MAPINFOPATHF, (char *)path) != 1) {
            fprintf(stderr, "invalid path '%s'\n", argv[i]);
            break;
          }

          if (strcmp(m.path, path) == 0)
            print_map_region(fmem, &m);
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

bool startswith(const char *str, const char *pre) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

void parse_map_line(Mapinfo *m, const char *line) {
  int n = sscanf(line, "%lx-%lx %4s %lx %lx:%lx %d " MAPINFOPATHF, &m->start,
                 &m->end, (char *)&m->mode, &m->offset, &m->major, &m->minor,
                 &m->inode, (char *)&m->path);

  if (n < 7)
    die("could not parse '%s'", line);

  if (n == 7)
    m->path[0] = '\0';
}

void print_map_region(FILE *f, const Mapinfo *m) {
  if (fseek(f, m->start, SEEK_SET) != 0) {
    fprintf(stderr, "could not read region %s %lx-%lx\n", m->path, m->start,
            m->end);
    return;
  }

  char buf[2048];
  size_t total = m->end - m->start;
  size_t current = 0;

  while (current <= total) {
    size_t n = MIN(sizeof(buf), total - current);
    if (n == 0)
      break;

    if (fread(buf, n, 1, f) != 1) {
      fprintf(stderr, "could not read region %s %lx-%lx chunk %lx-%lx\n",
              m->path, m->start, m->end, m->start + current,
              m->start + current + n);
      return;
    }

    current += n;
    efwrite(buf, n, 1, stdout);
  }
}

void usage(int argc, char **argv, int status) {
  (void)argc;
  fprintf(stderr,
          "usage: %s PID [OPTIONS] REGIONS...\n"
          "read map REGIONS of PID and writes to stdout\n"
          "REGIONS\n"
          "\t<addr start>-<addr end>\n"
          "\tall                        read all REGIONS\n"
          "\tinode:<inodeid>            read REGION by inodeid\n"
          "\tpath:<path>                read REGION by path\n"
          "\tdev:<majorid>:<minorid>    read REGION by dev\n"
          "\tmajor:<majorid>            read REGION by majorid\n"
          "\tminor:<minorid>            read REGION by minorid\n"
          "OPTIONS\n"
          "\t-h, --help                 shows usage and exit\n"
          "\tlist                       list all REGIONS and exit\n"
          "examples\n"
          "\t%s PID 7ff14e581000-7ff14e584000\n"
          "\t%s PID all\n"
          "\t%s PID path:[heap] path:[stack] path:/path/to/file\n"
          "\t%s PID inode:0\n",
          argv[0], argv[0], argv[0], argv[0], argv[0]);
  exit(status);
}

void popv(int *argc, char **argv, int index) {
  assert(index < *argc);

  if (index != *argc - 1) {
    for (int i = index; i < *argc + 1; i++)
      argv[i] = argv[i + 1];
  }
  (*argc)--;
}
