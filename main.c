#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "lib/parson.h"

#ifndef DEBUG
#define DEBUG 1
#endif

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s)
{
  s->len = 0;
  s->ptr = malloc(s->len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc failed; out of memory\n");
    exit(1);
  }

  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc failed; out of memory\n");
    exit(1);
  }

  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size * nmemb;
}

int request(CURL *curl, char *url, struct string s)
{
  curl_easy_setopt(curl, CURLOPT_URL, "osmc.local:3000/lights");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

  return curl_easy_perform(curl);
}

void getLights()
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl) {
    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, "osmc.local:3000/lights");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    /*curl_easy_setopt(curl, CURLOPT_VERBOSE, DEBUG);*/

    res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
      long response_code;;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

      if (response_code == 500) {
        printf("The server responded 500 (Internal Server Error)");
      }
    } else {
      fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    }

    JSON_Value *json_response = json_parse_string(s.ptr);
    char *serialized = json_serialize_to_string_pretty(json_response);
    puts(serialized);

    json_free_serialized_string(serialized);
    json_value_free(json_response);

    free(s.ptr);

    curl_easy_cleanup(curl);
  }
}

int main(void)
{
  getLights();
  return 0;
}
