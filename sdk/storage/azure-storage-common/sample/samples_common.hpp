// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include <functional>
#include <map>
#include <string>

const std::string& GetConnectionString();

class Sample {
public:
  static const std::map<std::string, std::function<void()>>& samples() { return m_samples(); }

protected:
  static void add_sample(std::string sample_name, std::function<void()> func)
  {
    m_samples().emplace(std::move(sample_name), std::move(func));
  }

private:
  static std::map<std::string, std::function<void()>>& m_samples()
  {
    static std::map<std::string, std::function<void()>> samples_instance;
    return samples_instance;
  }
};

#define SAMPLE(NAME, FUNCTION) \
  void FUNCTION(); \
\
  class Sample##NAME : public Sample { \
  public: \
    Sample##NAME() { add_sample(#NAME, FUNCTION); } \
  }; \
  namespace { \
    Sample##NAME Sample##NAME_; \
  }
