//   _____                     _ _
//  | ____|_ __   ___ ___   __| (_)_ __   __ _
//  |  _| | '_ \ / __/ _ \ / _` | | '_ \ / _` |
//  | |___| | | | (_| (_) | (_| | | | | | (_| |
//  |_____|_| |_|\___\___/ \__,_|_|_| |_|\__, |
//                                       |___/
// Binary encoding/decoding of data structures
// A data structure (struct) may often need to be moved to a different machine,
// either via file or via network. Unfortunately, the binary representation of
// the same structure on different architectures may be different.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// By default, structures are mapped in memory as ALIGNED, i.e. fields shorter
// than the native int size are zero-padded. This has two consequences when
// sending in binary format via network or files:
// 1. waste of space
// 2. different architecture may have different native int size and pad fields
//    differently, so that the binary copy cannot be correctly interpretated.
// The packed attribute instructs the compiler to avoid byte alignment and
// always pack the struct to its minimum size. For brevity we define a macro:
#define _packed __attribute__((packed))
// #define _packed __attribute(())
// try to swap the above two lines and see the difference in output.

// Chunk size for reallocating dynamic arrays
#define SCORES_BLOCKS 10

//   ____  _                   _           _     _           _   
//  / ___|| |_ _ __ _   _  ___| |_    ___ | |__ (_) ___  ___| |_ 
//  \___ \| __| '__| | | |/ __| __|  / _ \| '_ \| |/ _ \/ __| __|
//   ___) | |_| |  | |_| | (__| |_  | (_) | |_) | |  __/ (__| |_ 
//  |____/ \__|_|   \__,_|\___|\__|  \___/|_.__// |\___|\___|\__|
//                                            |__/               
// We define a data structure with some static fields (i.e. of constant size)
// and some with dynamic size. We put first the static fields and leave all the
// dynamic ones at the end. We group the static fields in a "header", which also
// holds the actual size of the dynamic fields.
typedef struct {
  // static part: no pointers here
  struct header_st {
    uint16_t id;
    unsigned char n;
    float value;
    // sizes of dynamically allocated fields
    uint32_t name_len, scores_len;
  } _packed header; // notice the _packed macro
  //  from now on, only pointers to dynamically allocated arrays
  char *name;
  double *scores;
} data_t;

//   __  __      _   _               _     
//  |  \/  | ___| |_| |__   ___   __| |___ 
//  | |\/| |/ _ \ __| '_ \ / _ \ / _` / __|
//  | |  | |  __/ |_| | | | (_) | (_| \__ \
//  |_|  |_|\___|\__|_| |_|\___/ \__,_|___/
                                        
// data object creator
data_t *data_new(unsigned char n, float v, char const *name) {
  static uint16_t id = 1;
  data_t *d = malloc(sizeof(data_t));
  if (!d) goto end;
  
  // static fields
  d->header.id = id++;
  d->header.n = n;
  d->header.value = v;
  
  // dynamic fields
  d->name = malloc(strlen(name) * sizeof(char) + 1);
  if (!d->name) {
    free(d);
    d = NULL;
    goto end;
  }
  strncpy(d->name, name, strlen(name));

  // We pre-alloc space for SCORES_BLOCKS elements
  d->scores = calloc(sizeof(*(d->scores)), SCORES_BLOCKS);
  if (!d->scores) {
    free(d);
    d = NULL;
    goto end;
  }
  memset(d->scores, 0, sizeof(*(d->scores)) * SCORES_BLOCKS);

  // update sizes of dynamic fields
  d->header.name_len = strlen(name) + 1;
  d->header.scores_len = 0;

end:
  return d;
}

// Deallocate a data object
void data_free(data_t *d) {
  free(d->name);
  free(d->scores);
  free(d);
}

// Append a value to data field scores
data_t *data_push(data_t *d, double val) {
  uint32_t l = d->header.scores_len;
  // if the current size divided by SCORES_BLOCKS has a remainder 0, then we
  // need to reallocate memory (grow array)
  if (l > 0 && l % SCORES_BLOCKS == 0) {
    d->scores = realloc(d->scores, sizeof(*(d->scores)) * (l + SCORES_BLOCKS));
  }
  d->scores[l] = val;
  d->header.scores_len++;
  return d;
}

// Print a description of data
void data_print(data_t *d) {
  size_t i = 0;
  if (!d)
    return;
  printf("data \"%s\": (address %p)\n", d->name, d);
  printf("  id: %d, n: %d, value: %.3f\n", d->header.id, d->header.n,
         d->header.value);
  if (d->header.scores_len > 0) {
    printf("  scores: [");
    for (i = 0; i < d->header.scores_len; i++) {
      printf("%.3lf, ", d->scores[i]);
    }
    printf("\b\b] \n");
  }
}

// Convert the data structure into an array of bytes
size_t data_to_buffer(data_t *d, char **buf) {
  // calculate total space needed and allocate memory
  size_t buflen = sizeof(d->header) + d->header.name_len +
                  d->header.scores_len * sizeof(*(d->scores));
  char *head = *buf = calloc(sizeof(char), buflen);
  if (!head)
    goto error;

  // Copy header as bytes
  memcpy(head, &d->header, sizeof(d->header));

  // advance insertion point, then copy first dynamic field
  head += sizeof(d->header);
  memcpy(head, d->name, d->header.name_len);

  // advance insertion point, then copy second dynamic field
  head += d->header.name_len;
  memcpy(head, d->scores, d->header.scores_len * sizeof(*(d->scores)));

  return buflen;
error:
  return 0;
}

// Create a new data object from a byte buffer
data_t *buffer_to_data(char *buf, size_t len) {
  char *head = buf;
  data_t *d = data_new(0, 0, "");
  if (!d)
    goto error;

  // copy header part
  memcpy(&d->header, buf, sizeof(d->header));

  // allocate first dynamic field and copy content from buffer
  d->name = calloc(sizeof(*(d->name)), d->header.name_len);
  if (!d->name)
    goto error;
  head += sizeof(d->header);
  memcpy(d->name, head, d->header.name_len);

  // allocate second dynamic field and copy content from buffer
  d->scores = calloc(sizeof(*(d->scores)), d->header.scores_len);
  if (!d->scores)
    goto error;
  memset(d->scores, 0, d->header.scores_len * sizeof(*(d->scores)));
  head += d->header.name_len;
  memcpy(d->scores, head, d->header.scores_len * sizeof(*(d->scores)));

  return d;
error:
  if (d->name)
    free(d->name);
  if (d)
    free(d);
  return NULL;
}

// Print a binary buffer in blocks of 4 bytes, 4 blocks per row
void buffer_print(char const *buffer, size_t len) {
  size_t i = 0;
  for (i = 0; i < len; i++) {
    printf("%02X", (unsigned char)buffer[i]);
    if (i % 4 == 3)
      printf(" ");
    if (i % 16 == 15)
      printf("\n");
  }
  printf("\n");
}


//   __  __       _          __                  _   _             
//  |  \/  | __ _(_)_ __    / _|_   _ _ __   ___| |_(_) ___  _ __  
//  | |\/| |/ _` | | '_ \  | |_| | | | '_ \ / __| __| |/ _ \| '_ \
//  | |  | | (_| | | | | | |  _| |_| | | | | (__| |_| | (_) | | | |
//  |_|  |_|\__,_|_|_| |_| |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|
                                                                
int main() {
  char *buffer;
  size_t buflen;
  data_t *data = data_new(17, 4, "test");
  data_t *data_out = NULL;
  data_push(data, 1);
  data_push(data, 2.5);
  data_push(data, 3.8);

  printf("Data:\n");
  data_print(data);

  printf("\nBuffer:\n");
  buflen = data_to_buffer(data, &buffer);
  buffer_print(buffer, buflen);

  printf("\nData out:\n");
  data_out = buffer_to_data(buffer, buflen);
  data_print(data_out);
  
  free(buffer);
  data_free(data);
  data_free(data_out);
  return 0;
}

// NOTE: there is one more gotcha in exchanging binary data among different
// platforms. Some platform store (int16_t)18 as hex "0012", some as "1200". The
// former are called BIG ENDIAN (MSD is at the smallest memory address), the
// latter are called LITTLE ENDIAN (LSD at the smallest memory address).

// This only applies to int types with more than 8 bits. Floats and doubles
// follow IEEE 754 on all platforms.

// Consequently, before encoding any integer it must be turned from host byte
// order (HBO) into a common, reference endianness, called NETWORK BYTE ORDER
// (NBO), which is big endian. Then, upon decoding, the byte order must be
// changed back from NBO to HBO.

// The following functions are available by including #include <arpa/inet.h>:
// - htons(), htonl() and htonll(): convert a 16, 32, 64 bit integer from HBO
//   (whichever) to NBO. If the host is also big endian, these functions do
//   nothing.
// - ntohs(), ntohl(), and ntohll(): the opposite, network-to-host. 

// In the data_to_buffer() function, all the numeric fields shall be hton before
// copying them into the buffer, then reverted back via ntoh. In the
// buffer_to_data() function, upon completing the copy of buffer content into
// the new structure, all the integer fields shall be ntoh-ed. If you have an
// array of integers, it must be ntoh-ed one element at a time.

// This is left as an exercise to the reader.