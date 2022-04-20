#include "azure/core/tracing/tracing.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"

namespace Azure { namespace Core { namespace Tracing {

  const SpanKind SpanKind::Internal("Internal");
  const SpanKind SpanKind::Client("Client");
  const SpanKind SpanKind::Consumer("Consumer");
  const SpanKind SpanKind::Producer("Producer");
  const SpanKind SpanKind::Server("Server");

  const SpanStatus SpanStatus::Unset("Unset");
  const SpanStatus SpanStatus::Ok("Ok");
  const SpanStatus SpanStatus::Error("Error");

}}} // namespace Azure::Core::Tracing

namespace Azure { namespace Core { namespace Tracing { namespace _internal {
  std::pair<Azure::Core::Context, ServiceSpan> ServiceTracing::CreateSpan(
      std::string const& methodName,
      Azure::Core::Context context)
  {
    methodName;
    context;
    return {};
  }

}}}} // namespace Azure::Core::Tracing::_internal