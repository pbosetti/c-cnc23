#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../toml.h"
#include "../defines.h"

void error(const char *msg, const char *desc) {
  fprintf(stderr, BRED "*** ERROR: %s - %s\n" CRESET, msg, desc? desc : "");
}

int main(int argc, char **argv) {
  FILE *fp = NULL;
  toml_table_t *conf = NULL;
  char errbuf[256];
  int err_c = 0;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    return 1;
  }

  fp = fopen(argv[1], "r");
  if (!fp) {
    error("fopen", strerror(errno));
    return 1;
  }
  conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
  fclose(fp);
  if (!conf) {
    error("toml_parse_file", errbuf);
    return 1;
  }

  //   __  __  ___ _____ _____ 
  //  |  \/  |/ _ \_   _|_   _|
  //  | |\/| | | | || |   | |  
  //  | |  | | |_| || |   | |  
  //  |_|  |_|\__\_\|_|   |_|  
                            
  {
    toml_table_t *mqtt = toml_table_in(conf, "MQTT");
    if (!mqtt) {
      error("Missing MQTT section", NULL);
      return 1;
    }
    printf(BGRN"*** Section [MQTT]\n"CRESET);

    toml_datum_t broker_addr = toml_string_in(mqtt, "broker_addr");
    if (!broker_addr.ok) {
      error("Missing broker_addr", NULL);
      err_c++;
    }
    toml_datum_t broker_port = toml_int_in(mqtt, "broker_port");
    if (!broker_port.ok) {
      error("Missing broker_port", NULL);
      err_c++;
    }
    printf(BBLK"Broker: "CRESET"%s:%lld\n", broker_addr.u.s, broker_port.u.i);
    
    toml_datum_t pub_topic = toml_string_in(mqtt, "pub_topic");
    if (!pub_topic.ok) {
      error("Missing pub_topic", NULL);
      err_c++;
    }
    toml_datum_t sub_topic = toml_string_in(mqtt, "sub_topic");
    if (!sub_topic.ok) {
      error("Missing sub_topic", NULL);
      err_c++;
    }
    printf(BBLK"Topics: "CRESET"PUB %s, SUB %s\n", pub_topic.u.s, sub_topic.u.s);
    
    toml_datum_t delay = toml_int_in(mqtt, "delay");
    if (!delay.ok) {
      error("Missing delay", NULL);
      err_c++;
    }
    printf(BBLK"Delay:  "CRESET"%lld\n", delay.u.i);
  }


  //    ____       ____ _   _  ____ 
  //   / ___|     / ___| \ | |/ ___|
  //  | |   _____| |   |  \| | |    
  //  | |__|_____| |___| |\  | |___ 
  //   \____|     \____|_| \_|\____|
                                 
  {
    toml_table_t *ccnc = toml_table_in(conf, "C-CNC");
    if (!ccnc) {
      error("Missing C-CNC section", NULL);
      return 1;
    }
    printf(BGRN"*** Section C-CNC\n"CRESET);

    toml_datum_t A = toml_double_in(ccnc, "A");
    if (!A.ok) {
      error("Missing A", NULL);
      err_c++;
    }
    printf(BBLK"A:         "CRESET"%f\n", A.u.d);

    toml_datum_t max_error = toml_double_in(ccnc, "max_error");
    if (!max_error.ok) {
      error("Missing max_error", NULL);
      err_c++;
    }
    printf(BBLK"max_error: "CRESET"%f\n", max_error.u.d);

    toml_datum_t tq = toml_double_in(ccnc, "tq");
    if (!tq.ok) {
      error("Missing tq", NULL);
      err_c++;
    }
    printf(BBLK"tq:        "CRESET"%f\n", tq.u.d);

    toml_datum_t rt_pacing = toml_double_in(ccnc, "rt_pacing");
    if (!rt_pacing.ok) {
      error("Missing rt_pacing", NULL);
      err_c++;
    }
    printf(BBLK"rt_pacing: "CRESET"%f\n", rt_pacing.u.d);
    
    toml_array_t *origin = toml_array_in(ccnc, "origin");
    if (toml_array_nelem(origin) != 3) {
      error("Origin must be an array of length 3", NULL);
    }
    toml_datum_t v;
    int i;
    printf(BBLK"Origin:    "CRESET"[");
    for (i=0; i<3; i++) {
      v = toml_double_at(origin, i);
      printf("%.3f, ", v.u.d);
    }
    printf("\b\b]\n");

    toml_array_t *offset = toml_array_in(ccnc, "offset");
    if (toml_array_nelem(offset) != 3) {
      error("Offset must be an array of length 3", NULL);
    }
    printf(BBLK"Offset:    "CRESET"[");
    for (i=0; i<3; i++) {
      v = toml_double_at(offset, i);
      printf("%.3f, ", v.u.d);
    }
    printf("\b\b]\n");
  }


  if (err_c > 0) {
    return 1;
  }
  return 0;
}