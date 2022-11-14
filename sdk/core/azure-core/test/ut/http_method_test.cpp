//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/http/http.hpp>

using namespace Azure::Core::Http;

// cSpell:words humuhumunukunukuapuaa

TEST(HttpMethod, Get)
{
  HttpMethod const get = HttpMethod::Get;
  EXPECT_EQ(get.ToString(), "GET");

  EXPECT_EQ(HttpMethod::Get, HttpMethod::Get);
  EXPECT_EQ(get, get);
  EXPECT_EQ(get, HttpMethod("GET"));
  EXPECT_EQ(HttpMethod("GET"), HttpMethod::Get);

  EXPECT_NE(get, HttpMethod("Get"));
  EXPECT_NE(HttpMethod("Get"), get);

  EXPECT_NE(get, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), get);

  EXPECT_EQ(get, HttpMethod::Get);
  EXPECT_EQ(HttpMethod::Get, get);

  EXPECT_NE(get, HttpMethod::Head);
  EXPECT_NE(get, HttpMethod::Post);
  EXPECT_NE(get, HttpMethod::Put);
  EXPECT_NE(get, HttpMethod::Delete);
  EXPECT_NE(get, HttpMethod::Patch);
  EXPECT_NE(get, HttpMethod("TRACE"));
  EXPECT_NE(get, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Head, get);
  EXPECT_NE(HttpMethod::Post, get);
  EXPECT_NE(HttpMethod::Put, get);
  EXPECT_NE(HttpMethod::Delete, get);
  EXPECT_NE(HttpMethod::Patch, get);
  EXPECT_NE(HttpMethod("TRACE"), get);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), get);
}

TEST(HttpMethod, Head)
{
  HttpMethod const head = HttpMethod::Head;
  EXPECT_EQ(head.ToString(), "HEAD");

  EXPECT_EQ(HttpMethod::Head, HttpMethod::Head);
  EXPECT_EQ(head, head);
  EXPECT_EQ(head, HttpMethod("HEAD"));
  EXPECT_EQ(HttpMethod("HEAD"), HttpMethod::Head);

  EXPECT_NE(head, HttpMethod("Head"));
  EXPECT_NE(HttpMethod("Head"), head);

  EXPECT_NE(head, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), head);

  EXPECT_EQ(head, HttpMethod::Head);
  EXPECT_EQ(HttpMethod::Head, head);

  EXPECT_NE(head, HttpMethod::Get);
  EXPECT_NE(head, HttpMethod::Post);
  EXPECT_NE(head, HttpMethod::Put);
  EXPECT_NE(head, HttpMethod::Delete);
  EXPECT_NE(head, HttpMethod::Patch);
  EXPECT_NE(head, HttpMethod("TRACE"));
  EXPECT_NE(head, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Get, head);
  EXPECT_NE(HttpMethod::Post, head);
  EXPECT_NE(HttpMethod::Put, head);
  EXPECT_NE(HttpMethod::Delete, head);
  EXPECT_NE(HttpMethod::Patch, head);
  EXPECT_NE(HttpMethod("TRACE"), head);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), head);
}

TEST(HttpMethod, Post)
{
  HttpMethod const post = HttpMethod::Post;
  EXPECT_EQ(post.ToString(), "POST");

  EXPECT_EQ(HttpMethod::Post, HttpMethod::Post);
  EXPECT_EQ(post, post);
  EXPECT_EQ(post, HttpMethod("POST"));
  EXPECT_EQ(HttpMethod("POST"), HttpMethod::Post);

  EXPECT_NE(post, HttpMethod("Post"));
  EXPECT_NE(HttpMethod("Post"), post);

  EXPECT_NE(post, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), post);

  EXPECT_EQ(post, HttpMethod::Post);
  EXPECT_EQ(HttpMethod::Post, post);

  EXPECT_NE(post, HttpMethod::Get);
  EXPECT_NE(post, HttpMethod::Head);
  EXPECT_NE(post, HttpMethod::Put);
  EXPECT_NE(post, HttpMethod::Delete);
  EXPECT_NE(post, HttpMethod::Patch);
  EXPECT_NE(post, HttpMethod("TRACE"));
  EXPECT_NE(post, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Get, post);
  EXPECT_NE(HttpMethod::Head, post);
  EXPECT_NE(HttpMethod::Put, post);
  EXPECT_NE(HttpMethod::Delete, post);
  EXPECT_NE(HttpMethod::Patch, post);
  EXPECT_NE(HttpMethod("TRACE"), post);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), post);
}

TEST(HttpMethod, Put)
{
  HttpMethod const put = HttpMethod::Put;
  EXPECT_EQ(put.ToString(), "PUT");

  EXPECT_EQ(HttpMethod::Put, HttpMethod::Put);
  EXPECT_EQ(put, put);
  EXPECT_EQ(put, HttpMethod("PUT"));
  EXPECT_EQ(HttpMethod("PUT"), HttpMethod::Put);

  EXPECT_NE(put, HttpMethod("Put"));
  EXPECT_NE(HttpMethod("Put"), put);

  EXPECT_NE(put, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), put);

  EXPECT_EQ(put, HttpMethod::Put);
  EXPECT_EQ(HttpMethod::Put, put);

  EXPECT_NE(put, HttpMethod::Get);
  EXPECT_NE(put, HttpMethod::Head);
  EXPECT_NE(put, HttpMethod::Post);
  EXPECT_NE(put, HttpMethod::Delete);
  EXPECT_NE(put, HttpMethod::Patch);
  EXPECT_NE(put, HttpMethod("TRACE"));
  EXPECT_NE(put, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Get, put);
  EXPECT_NE(HttpMethod::Head, put);
  EXPECT_NE(HttpMethod::Post, put);
  EXPECT_NE(HttpMethod::Delete, put);
  EXPECT_NE(HttpMethod::Patch, put);
  EXPECT_NE(HttpMethod("TRACE"), put);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), put);
}

TEST(HttpMethod, Delete)
{
  HttpMethod const delete_ = HttpMethod::Delete;
  EXPECT_EQ(delete_.ToString(), "DELETE");

  EXPECT_EQ(HttpMethod::Delete, HttpMethod::Delete);
  EXPECT_EQ(delete_, delete_);
  EXPECT_EQ(delete_, HttpMethod("DELETE"));
  EXPECT_EQ(HttpMethod("DELETE"), HttpMethod::Delete);

  EXPECT_NE(delete_, HttpMethod("Put"));
  EXPECT_NE(HttpMethod("Put"), delete_);

  EXPECT_NE(delete_, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), delete_);

  EXPECT_EQ(delete_, HttpMethod::Delete);
  EXPECT_EQ(HttpMethod::Delete, delete_);

  EXPECT_NE(delete_, HttpMethod::Get);
  EXPECT_NE(delete_, HttpMethod::Head);
  EXPECT_NE(delete_, HttpMethod::Post);
  EXPECT_NE(delete_, HttpMethod::Put);
  EXPECT_NE(delete_, HttpMethod::Patch);
  EXPECT_NE(delete_, HttpMethod("TRACE"));
  EXPECT_NE(delete_, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Get, delete_);
  EXPECT_NE(HttpMethod::Head, delete_);
  EXPECT_NE(HttpMethod::Post, delete_);
  EXPECT_NE(HttpMethod::Put, delete_);
  EXPECT_NE(HttpMethod::Patch, delete_);
  EXPECT_NE(HttpMethod("TRACE"), delete_);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), delete_);
}

TEST(HttpMethod, Patch)
{
  HttpMethod const patch = HttpMethod::Patch;
  EXPECT_EQ(patch.ToString(), "PATCH");

  EXPECT_EQ(HttpMethod::Patch, HttpMethod::Patch);
  EXPECT_EQ(patch, patch);
  EXPECT_EQ(patch, HttpMethod("PATCH"));
  EXPECT_EQ(HttpMethod("PATCH"), HttpMethod::Patch);

  EXPECT_NE(patch, HttpMethod("Patch"));
  EXPECT_NE(HttpMethod("Patch"), patch);

  EXPECT_NE(patch, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), patch);

  EXPECT_EQ(patch, HttpMethod::Patch);
  EXPECT_EQ(HttpMethod::Patch, patch);

  EXPECT_NE(patch, HttpMethod::Get);
  EXPECT_NE(patch, HttpMethod::Head);
  EXPECT_NE(patch, HttpMethod::Post);
  EXPECT_NE(patch, HttpMethod::Put);
  EXPECT_NE(patch, HttpMethod::Delete);
  EXPECT_NE(patch, HttpMethod("TRACE"));
  EXPECT_NE(patch, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Get, patch);
  EXPECT_NE(HttpMethod::Head, patch);
  EXPECT_NE(HttpMethod::Post, patch);
  EXPECT_NE(HttpMethod::Put, patch);
  EXPECT_NE(HttpMethod::Delete, patch);
  EXPECT_NE(HttpMethod("TRACE"), patch);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), patch);
}

TEST(HttpMethod, Trace)
{
  HttpMethod const trace = HttpMethod("TRACE");
  EXPECT_EQ(trace.ToString(), "TRACE");

  EXPECT_EQ(trace, trace);
  EXPECT_EQ(trace, HttpMethod("TRACE"));
  EXPECT_EQ(HttpMethod("TRACE"), trace);

  EXPECT_NE(trace, HttpMethod("Trace"));
  EXPECT_NE(HttpMethod("Trace"), trace);

  EXPECT_NE(trace, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), trace);

  EXPECT_NE(trace, HttpMethod::Get);
  EXPECT_NE(trace, HttpMethod::Head);
  EXPECT_NE(trace, HttpMethod::Post);
  EXPECT_NE(trace, HttpMethod::Put);
  EXPECT_NE(trace, HttpMethod::Delete);
  EXPECT_NE(trace, HttpMethod::Patch);
  EXPECT_NE(trace, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));

  EXPECT_NE(HttpMethod::Get, trace);
  EXPECT_NE(HttpMethod::Head, trace);
  EXPECT_NE(HttpMethod::Post, trace);
  EXPECT_NE(HttpMethod::Put, trace);
  EXPECT_NE(HttpMethod::Delete, trace);
  EXPECT_NE(HttpMethod::Patch, trace);
  EXPECT_NE(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), trace);
}

TEST(HttpMethod, HawaiianFish)
{
  HttpMethod const humuhumunukunukuapuaa = HttpMethod("HUMUHUMUNUKUNUKUAPUAA");
  EXPECT_EQ(humuhumunukunukuapuaa.ToString(), "HUMUHUMUNUKUNUKUAPUAA");

  EXPECT_EQ(humuhumunukunukuapuaa, humuhumunukunukuapuaa);
  EXPECT_EQ(humuhumunukunukuapuaa, HttpMethod("HUMUHUMUNUKUNUKUAPUAA"));
  EXPECT_EQ(HttpMethod("HUMUHUMUNUKUNUKUAPUAA"), humuhumunukunukuapuaa);

  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod("Humuhumunukunukuapuaa"));
  EXPECT_NE(HttpMethod("Humuhumunukunukuapuaa"), humuhumunukunukuapuaa);

  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod(std::string()));
  EXPECT_NE(HttpMethod(std::string()), humuhumunukunukuapuaa);

  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod::Get);
  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod::Head);
  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod::Post);
  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod::Put);
  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod::Delete);
  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod::Patch);
  EXPECT_NE(humuhumunukunukuapuaa, HttpMethod("TRACE"));

  EXPECT_NE(HttpMethod::Get, humuhumunukunukuapuaa);
  EXPECT_NE(HttpMethod::Head, humuhumunukunukuapuaa);
  EXPECT_NE(HttpMethod::Post, humuhumunukunukuapuaa);
  EXPECT_NE(HttpMethod::Put, humuhumunukunukuapuaa);
  EXPECT_NE(HttpMethod::Delete, humuhumunukunukuapuaa);
  EXPECT_NE(HttpMethod::Patch, humuhumunukunukuapuaa);
  EXPECT_NE(HttpMethod("TRACE"), humuhumunukunukuapuaa);
}
