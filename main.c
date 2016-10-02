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
  puts("init_string()");
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

CURLcode request(CURL *curl, char *url, struct string *s)
{
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
  /*curl_easy_setopt(curl, CURLOPT_VERBOSE, DEBUG);*/

  return curl_easy_perform(curl);
}

void getLights()
{
  CURL *curl;
  CURLcode response;

  curl = curl_easy_init();
  if (curl) {
    struct string s;
    init_string(&s);

    puts("Request begin");
    response = request(curl, "osmc.local:3000/lights", &s);
    puts("Request end");

    if (response == CURLE_OK) {
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      printf("The response code was %ld\n", response_code);

      double request_time;
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &request_time);
      printf("The request took %f seconds to complete\n", request_time);

      if (response_code == 500) {
        puts("The server responded 500 (Internal Server Error)\n");
      }
    } else {
      fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(response));
    }

    JSON_Value *json_response = json_parse_string(s.ptr);
    char *serialized = json_serialize_to_string_pretty(json_response);
    puts(serialized);

    puts("Cleanup serialized start");
    json_free_serialized_string(serialized);
    puts("Cleanup serialized done");
    puts("Cleanup json_response start");
    json_value_free(json_response);
    puts("Cleanup json_response done");

    puts("Cleanup s.ptr start");
    free(s.ptr);
    puts("Cleanup s.ptr done");

    curl_easy_cleanup(curl);
  }
}

int main(void)
{
  getLights();
  return 0;
}
