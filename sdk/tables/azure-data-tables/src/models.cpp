#include "azure/data/tables/models.hpp"
namespace Azure { namespace Data { namespace Tables { namespace Models {
  const GeoReplicationStatus GeoReplicationStatus::Live("live");
  const GeoReplicationStatus GeoReplicationStatus::Bootstrap("bootstrap");
  const GeoReplicationStatus GeoReplicationStatus::Unavailable("unavailable");

  const TableEntityDataType TableEntityDataType::EdmBinary("Edm.Binary");
  const TableEntityDataType TableEntityDataType::EdmBoolean("Edm.Boolean");
  const TableEntityDataType TableEntityDataType::EdmDateTime("Edm.DateTime");
  const TableEntityDataType TableEntityDataType::EdmDouble("Edm.Double");
  const TableEntityDataType TableEntityDataType::EdmGuid("Edm.Guid");
  const TableEntityDataType TableEntityDataType::EdmInt32("Edm.Int32");
  const TableEntityDataType TableEntityDataType::EdmInt64("Edm.Int64");
  const TableEntityDataType TableEntityDataType::EdmString("Edm.String");
}}}} // namespace Azure::Data::Tables::Models
