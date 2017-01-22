#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h>
#include "../lib/parson.h"

#define API_BASE "osmc.local:3000"

#define DEBUG 1

CURL *curl;

struct string {
  char *ptr;
  size_t len;
};

struct light {
  int number;
  unsigned int brightness;
  unsigned int hue;
  unsigned int saturation;
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

CURLcode connection(char *method, char *path, struct string *response_body)
{
  static char *host = API_BASE;
  char url[strlen(host) + strlen(path) + 1];
  snprintf(url, sizeof url, "%s%s", host, path);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_body);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, DEBUG);

  CURLcode response = curl_easy_perform(curl);

  if (response == CURLE_OK) {
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    printf("The server responeded %ld\n", response_code);

    if (response_code == 500) {
      puts("The server responded 500 (Internal Server Error)\n");
    }
  } else {
    fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(response));
  }

  double request_time;
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &request_time);
  printf("The request took %f seconds to complete\n", request_time);

  return response;
}


void init_curl()
{
  curl = curl_easy_init();
  if (!curl) {
    fprintf(stderr, "Failed to initialize curl");
    curl_easy_cleanup(curl);
    exit(1);
  }
}

void getLights()
{
  struct string response_body;
  init_string(&response_body);

  puts("Request begin");
  CURLcode response = connection("GET", "/lights", &response_body);
  puts("Request end");

  if (response == CURLE_OK) {
    JSON_Value *json_response = json_parse_string(response_body.ptr);
    char *serialized = json_serialize_to_string_pretty(json_response);
    puts(serialized);

    puts("Cleanup serialized start");
    json_free_serialized_string(serialized);
    puts("Cleanup serialized done");
    puts("Cleanup json_response start");
    json_value_free(json_response);
    puts("Cleanup json_response done");
  }

  curl_easy_cleanup(curl);
  puts("Cleanup response_body.ptr start");
  free(response_body.ptr);
  puts("Cleanup response_body.ptr done");
}

int int_len(int value)
{
  return floor(log10(abs(value))) + 1;
}

void set_lights(struct light *light)
{
  struct string response_body;
  init_string(&response_body);

  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;

  json_object_set_number(root_object, "brightness", light->brightness);
  serialized_string = json_serialize_to_string(root_value);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, serialized_string);

  char *header_start = "Content-Length: ";
  unsigned long header_end = strlen(serialized_string);
  char content_length[strlen(header_start) + int_len(header_end) + 1];
  snprintf(content_length, sizeof content_length, "%s%lu", header_start, header_end);

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, content_length);
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Accept: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  puts("Request begin");
  char *path_start = "/lights/";
  char path[strlen(path_start) + int_len(light->number) + 1];
  snprintf(path, sizeof path, "%s%d", path_start, light->number);
  CURLcode response = connection("POST", path, &response_body);
  puts("Request end");

  json_value_free(root_value);
  curl_slist_free_all(headers);
  json_free_serialized_string(serialized_string);

  if (response == CURLE_OK) {
    JSON_Value *json_response = json_parse_string(response_body.ptr);
    char *serialized = json_serialize_to_string_pretty(json_response);
    puts(serialized);

    puts("Cleanup serialized start");
    json_free_serialized_string(serialized);
    puts("Cleanup serialized done");
    puts("Cleanup json_response start");
    json_value_free(json_response);
    puts("Cleanup json_response done");
  }

  curl_easy_cleanup(curl);
  free(response_body.ptr);
}

int main(void)
{
  init_curl();

  struct light lamp;
  lamp.number = 1;
  lamp.brightness = 30;

  set_lights(&lamp);

  return 0;
}
