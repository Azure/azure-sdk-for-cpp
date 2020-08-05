// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Test speed of libcurl uploading 8Mb with easy_send() and without it.
 *
 */

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif // Windows

#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <algorithm>

#include <chrono>

#define UPLOAD_SIZE 8 * 1024 * 1024
#define CONTENT_LENGTH "Content-Length: "
#define CYCLE_COUNT 5

struct Span
{
  char* buffer;
  int64_t size;
  int64_t offset;
};

static size_t read_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
  size_t n = size * nitems;
  Span* data = (Span*)userdata;

  int64_t toCopy = std::min(n, static_cast<size_t>(data->size - data->offset));
  std::memcpy(buffer, data->buffer + data->offset, toCopy);
  data->offset += toCopy;

  return toCopy;
}

// just dismiss the answer
static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  (void)ptr;
  (void)userdata;

  size_t realsize = size * nmemb;
  return realsize;
}

int main()
{

  std::cout << "Size: " << UPLOAD_SIZE << ". Will run " << CYCLE_COUNT << " Times." << std::endl
            << std::flush;
  char* buffer = new char[UPLOAD_SIZE];

  std::string url = "https://httpbin.org/put";

  int64_t accumulator = 0;
  for (int64_t i = 1; i <= CYCLE_COUNT; i++)
  {
    CURL* easy_handle = curl_easy_init();

    curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_PUT, 1L);

    curl_easy_setopt(easy_handle, CURLOPT_URL, url.data());

    struct curl_slist* chunk = NULL;
    chunk = curl_slist_append(chunk, "x-ms-version: 2019-02-02");
    std::string content_length(CONTENT_LENGTH);
    content_length.append(std::to_string(UPLOAD_SIZE));
    chunk = curl_slist_append(chunk, content_length.data());
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, chunk);

    curl_easy_setopt(easy_handle, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
    auto span = Span();
    span.buffer = buffer;
    span.size = UPLOAD_SIZE;
    curl_easy_setopt(easy_handle, CURLOPT_READDATA, (void*)&span);
    curl_easy_setopt(easy_handle, CURLOPT_BUFFERSIZE, UPLOAD_SIZE);
    curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)UPLOAD_SIZE);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto r = curl_easy_perform(easy_handle);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    long response_code = 0;
    if (r == CURLE_OK)
    {
      curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
    }
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "Time difference (" << i << ") = " << time
              << "[ms]. Status code: " << response_code << std::endl;

    if (response_code == 200)
    {
      accumulator += time;
    }
    curl_easy_cleanup(easy_handle);
  }
  std::cout << std::endl << "Average: " << accumulator / CYCLE_COUNT << std::endl;

  delete[] buffer;
}
