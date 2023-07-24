#include "azure/messaging/eventhubs/models/partition_client_models.hpp"

#include "azure/messaging/eventhubs/models/management_models.hpp"

#include <iomanip>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  std::ostream& operator<<(std::ostream& os, StartPosition const& sp)
  {
    os << "StartPosition:[";
    if (sp.Offset.HasValue())
    {
      os << "Off: " << sp.Offset.Value() << std::endl;
    }
    if (sp.SequenceNumber.HasValue())
    {
      os << "Seq: " << sp.SequenceNumber.Value() << std::endl;
    }
    if (sp.EnqueuedTime.HasValue())
    {
      os << "Enq: " << sp.EnqueuedTime.Value().ToString();
    }
    os << " Inclusive: " << std::boolalpha << sp.Inclusive;
    if (sp.Earliest.HasValue())
    {
      os << " Earliest: " << std::boolalpha << sp.Earliest.Value() << std::endl;
    }
    if (sp.Latest.HasValue())
    {
      os << "Latest: " << std::boolalpha << sp.Latest.Value() << std::endl;
    }
    os << "]";

    return os;
  }
  std::ostream& operator<<(std::ostream& os, EventHubPartitionProperties const& pp)
  {
    os << "PartitionProperties:[[" << pp.Name << "]: ";
    os << "Id: " << pp.PartitionId << std::endl;
    os << "BeginningSequenceNumber: " << pp.BeginningSequenceNumber << std::endl;
    os << "LastEnqueuedSequenceNumber: " << pp.LastEnqueuedSequenceNumber << std::endl;
    os << "LastEnqueuedOffset: " << pp.LastEnqueuedOffset << std::endl;
    os << "LastEnqueuedTimeUtc: " << pp.LastEnqueuedTimeUtc.ToString() << std::endl;
    os << "IsEmpty" << std::boolalpha << pp.IsEmpty << std::endl;
    os << "]" << std::endl;

    return os;
  }
}}}} // namespace Azure::Messaging::EventHubs::Models
