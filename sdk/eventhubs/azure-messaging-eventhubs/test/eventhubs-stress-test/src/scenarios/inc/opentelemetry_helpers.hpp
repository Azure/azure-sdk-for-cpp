// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <utility>

#include <opentelemetry/logs/provider.h>
#include <opentelemetry/sdk/logs/logger.h>
#include <opentelemetry/sdk/trace/tracer.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/semantic_conventions.h>

opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> GetLogger();

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer();

std::pair<opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>, opentelemetry::trace::Scope>
CreateStressSpan(std::string const& name);
