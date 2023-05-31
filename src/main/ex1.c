//   _____                   _            _ 
//  | ____|_  _____ _ __ ___(_)___  ___  / |
//  |  _| \ \/ / _ \ '__/ __| / __|/ _ \ | |
//  | |___ >  <  __/ | | (__| \__ \  __/ | |
//  |_____/_/\_\___|_|  \___|_|___/\___| |_|
// Renumber and cleanup G-code file

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

int main(int argc, const char **argv) {
  FILE *file = NULL;
  size_t step = 1;
  char *line = NULL, *line_out = NULL, *word = NULL;
  size_t linecap = 0;
  ssize_t line_len = 0;
  int width = 1; // N0123 has width = 4
  size_t n = 0, i = 0; // counter for line numbers
  size_t max_line = 0, max_line_n;
  size_t min_line = -1, min_line_n;
  size_t nchar = 0;

  // check number of CLI arguments
  if (argc > 3 || argc < 2) {
    fprintf(stderr, "Usage: %s <G-code path> [step]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // if there's a second (optional) argument, read it as a step number
  if (3 == argc) {
    step = atoi(argv[2]);
  }

  // open the G-code file
  if ( !(file = fopen(argv[1], "r")) ) {
    fprintf(stderr, "Cannot open file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  // counting number of lines
  while (getline(&line, &linecap, file) >= 0) {
    n++;
  }
  // calculate width of the N field
  width = (int)ceil(log10(n * step));
  n = 0;
  
  // reset the file pointer to the beginning
  fseek(file, 0, SEEK_SET);

  // loop over the lines, changing onle line at a time
  while ((line_len = getline(&line, &linecap, file)) >= 0) {
    // remove the new line character (if present)
    if ('\n' == line[line_len - 1]) {
      line[line_len - 1] = '\0';
    }

    // prepare the output string
    // N1 -> N0001
    line_out = realloc(line_out, strlen(line) + 1 + width);
    memset(line_out, 0, strlen(line) + 1 + width);

    // always start with line number
    sprintf(line_out, "N%0*zu ", width, n * step);

    // split the line into tokens
    i = 0;
    while ((word = strsep(&line, " ")) != NULL) {
      if (toupper(word[0]) == 'N' || 0 == strlen(word)) {
        // skip the N command (already printed)
        continue;
      } else {
        sprintf(line_out + strlen(line_out), "%c%s ", toupper(word[0]), word + 1);
        i++;
      }
    }

    // skip if the (new) line is empty (zero fields)
    if (0 == i) { continue; }

    // print the current output line
    printf("%s\n", line_out);

    // calculate statistics
    line_len = strlen(line_out);
    if (line_len > max_line) {
      max_line = line_len;
      max_line_n = n;
    }
    if (line_len < min_line) {
      min_line = line_len;
      min_line_n = n;
    }

    // increment counters
    n++;
    nchar += line_len;
  }

  // print output statistics
  fprintf(stderr, "File %s has %zu lines and %zu characters\n", argv[1], n, nchar);
  fprintf(stderr, " Longest line: %zu (%zu chars)\n", max_line_n, max_line);
  fprintf(stderr, "Shortest line: %zu (%zu chars)\n", min_line_n, min_line);

  // free resources
  free(line_out);
  free(line);
  fclose(file);

  return EXIT_SUCCESS;
}