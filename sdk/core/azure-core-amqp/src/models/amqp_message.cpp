// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/models/amqp_message.hpp"

#include "../amqp/private/unique_handle.hpp"
#include "../models/private/header_impl.hpp"
#include "../models/private/message_impl.hpp"
#include "../models/private/properties_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/models/amqp_protocol.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_annotations.h>
#include <azure_uamqp_c/amqp_definitions_application_properties.h>
#include <azure_uamqp_c/amqp_definitions_footer.h>
#include <azure_uamqp_c/message.h>
#endif

#include <iostream>
#include <set>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  // @cond
  void UniqueHandleHelper<MESSAGE_INSTANCE_TAG>::FreeAmqpMessage(MESSAGE_HANDLE value)
  {
    message_destroy(value);
  }
// @endcond
#endif
}}}} // namespace Azure::Core::Amqp::_detail

using namespace Azure::Core::Amqp::_detail;
using namespace Azure::Core::Amqp::Models::_detail;

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  namespace {
#if ENABLE_UAMQP
    UniqueMessageHeaderHandle GetHeaderFromMessage(MESSAGE_HANDLE message)
    {
      if (message != nullptr)
      {
        HEADER_HANDLE headerValue;
        if (!message_get_header(message, &headerValue))
        {
          return UniqueHandle<HEADER_INSTANCE_TAG>(headerValue);
        }
      }
      return nullptr;
    }

    UniquePropertiesHandle GetPropertiesFromMessage(MESSAGE_HANDLE const& message)
    {
      if (message != nullptr)
      {
        PROPERTIES_HANDLE propertiesValue;
        if (!message_get_properties(message, &propertiesValue))
        {
          return UniqueHandle<PROPERTIES_INSTANCE_TAG>(propertiesValue);
        }
      }
      return nullptr;
    }
#endif // ENABLE_UAMQP
  } // namespace

#if ENABLE_UAMQP
  std::shared_ptr<AmqpMessage> _detail::AmqpMessageFactory::FromUamqp(MESSAGE_INSTANCE_TAG* message)
  {
    if (message == nullptr)
    {
      return nullptr;
    }
    auto rv{std::make_shared<AmqpMessage>()};
    rv->Header = _detail::MessageHeaderFactory::FromImplementation(GetHeaderFromMessage(message));
    rv->Properties
        = _detail::MessagePropertiesFactory::FromUamqp(GetPropertiesFromMessage(message));

    {
      delivery_annotations annotationsVal;
      // message_get_delivery_annotations returns a clone of the message annotations.
      if (!message_get_delivery_annotations(message, &annotationsVal) && annotationsVal != nullptr)
      {
        UniqueAmqpValueHandle deliveryAnnotations(annotationsVal);
        auto deliveryMap
            = Models::_detail::AmqpValueFactory::FromImplementation(deliveryAnnotations).AsMap();
        rv->DeliveryAnnotations = deliveryMap;
      }
    }
    {
      // message_get_message_annotations returns a clone of the message annotations.
      AMQP_VALUE messageAnnotations{};
      if (!message_get_message_annotations(message, &messageAnnotations) && messageAnnotations)
      {
        rv->MessageAnnotations = Models::_detail::AmqpValueFactory::FromImplementation(
                                     UniqueAmqpValueHandle{messageAnnotations})
                                     .AsMap();
      }
    }
    {
      /*
       * The ApplicationProperties field in an AMQP message for uAMQP expects that the map value
       * is wrapped as a described value. A described value has a ULONG descriptor value and a
       * value type.
       *
       * Making things even more interesting, the ApplicationProperties field in an uAMQP message
       * is asymmetric.
       *
       * The MessageSender class will wrap ApplicationProperties in a described value, so when
       * setting application properties, the described value must NOT be present, but when
       * decoding an application properties, the GetApplicationProperties method has to be able to
       * handle both when the described value is present or not.
       */
      AMQP_VALUE properties;
      if (!message_get_application_properties(message, &properties) && properties)
      {
        UniqueAmqpValueHandle describedProperties(properties);
        properties = nullptr;
        if (describedProperties)
        {
          AMQP_VALUE value;
          if (amqpvalue_get_type(describedProperties.get()) == AMQP_TYPE_DESCRIBED)
          {
            auto describedType = amqpvalue_get_inplace_descriptor(describedProperties.get());
            uint64_t describedTypeValue;
            if (amqpvalue_get_ulong(describedType, &describedTypeValue))
            {
              throw std::runtime_error("Could not retrieve application properties described type.");
            }
            if (describedTypeValue
                != static_cast<uint64_t>(
                    Azure::Core::Amqp::_detail::AmqpDescriptors::ApplicationProperties))
            {
              throw std::runtime_error("Application Properties are not the corect described type.");
            }

            value = amqpvalue_get_inplace_described_value(describedProperties.get());
          }
          else
          {
            value = describedProperties.get();
          }
          if (amqpvalue_get_type(value) != AMQP_TYPE_MAP)
          {
            throw std::runtime_error("Application Properties must be a map?!");
          }
          auto appProperties = AmqpMap(_detail::AmqpValueFactory::FromImplementation(
              _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)}));
          for (auto const& val : appProperties)
          {
            if (val.first.GetType() != AmqpValueType::String)
            {
              throw std::runtime_error("Key of Application Properties must be a string.");
            }
            rv->ApplicationProperties.emplace(
                std::make_pair(static_cast<std::string>(val.first), val.second));
          }
        }
      }
    }
    {
      AMQP_VALUE deliveryTagVal;
      if (!message_get_delivery_tag(message, &deliveryTagVal))
      {
        UniqueAmqpValueHandle deliveryTag(deliveryTagVal);
        rv->DeliveryTag = _detail::AmqpValueFactory::FromImplementation(deliveryTag);
      }
    }
    {
      annotations footerVal;
      if (!message_get_footer(message, &footerVal) && footerVal)
      {
        UniqueAmqpValueHandle footerAnnotations(footerVal);
        footerVal = nullptr;
        auto footerMap = _detail::AmqpValueFactory::FromImplementation(footerAnnotations).AsMap();
        rv->Footer = footerMap;
      }
    }
    {
      MESSAGE_BODY_TYPE bodyType;

      if (!message_get_body_type(message, &bodyType))
      {
        switch (bodyType)
        {
          case MESSAGE_BODY_TYPE_NONE:
            rv->BodyType = MessageBodyType::None;
            break;
          case MESSAGE_BODY_TYPE_DATA: {
            size_t dataCount;
            if (!message_get_body_amqp_data_count(message, &dataCount))
            {
              for (auto i = 0ul; i < dataCount; i += 1)
              {
                BINARY_DATA binaryValue;
                if (!message_get_body_amqp_data_in_place(message, i, &binaryValue))
                {
                  rv->m_binaryDataBody.push_back(AmqpBinaryData(std::vector<std::uint8_t>(
                      binaryValue.bytes, binaryValue.bytes + binaryValue.length)));
                }
              }
            }
            rv->BodyType = MessageBodyType::Data;
          }
          break;
          case MESSAGE_BODY_TYPE_SEQUENCE: {

            size_t sequenceCount;
            if (!message_get_body_amqp_sequence_count(message, &sequenceCount))
            {
              for (auto i = 0ul; i < sequenceCount; i += 1)
              {
                AMQP_VALUE sequence;
                if (!message_get_body_amqp_sequence_in_place(message, i, &sequence))
                {
                  rv->m_amqpSequenceBody.push_back(_detail::AmqpValueFactory::FromImplementation(
                      _detail::UniqueAmqpValueHandle{amqpvalue_clone(sequence)}));
                }
              }
            }
            rv->BodyType = MessageBodyType::Sequence;
          }
          break;
          case MESSAGE_BODY_TYPE_VALUE: {
            AMQP_VALUE bodyValue;
            if (!message_get_body_amqp_value_in_place(message, &bodyValue))
            {
              rv->m_amqpValueBody = _detail::AmqpValueFactory::FromImplementation(
                  _detail::UniqueAmqpValueHandle{amqpvalue_clone(bodyValue)});
            }
            rv->BodyType = MessageBodyType::Value;
          }
          break;
          case MESSAGE_BODY_TYPE_INVALID:
            throw std::runtime_error("Invalid message body type.");
          default:
            throw std::runtime_error("Unknown body type.");
        }
      }
    }
    return rv;
  }

  UniqueMessageHandle _detail::AmqpMessageFactory::ToUamqp(AmqpMessage const& message)
  {
    UniqueMessageHandle rv(message_create());

    // AMQP 1.0 specifies a message format of 0, but EventHubs uses other values.
    if (message_set_message_format(rv.get(), message.MessageFormat))
    {
      throw std::runtime_error("Could not set destination message format.");
    }

    if (message_set_header(
            rv.get(), _detail::MessageHeaderFactory::ToImplementation(message.Header).get()))
    {
      throw std::runtime_error("Could not set message header.");
    }
    if (message_set_properties(
            rv.get(), _detail::MessagePropertiesFactory::ToUamqp(message.Properties).get()))
    {
      throw std::runtime_error("Could not set message properties.");
    }

    if (!message.DeliveryAnnotations.empty())
    {
      if (message_set_delivery_annotations(
              rv.get(),
              _detail::AmqpValueFactory::ToImplementation(
                  message.DeliveryAnnotations.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set delivery annotations.");
      }
    }

    if (!message.MessageAnnotations.empty())
    {
      if (message_set_message_annotations(
              rv.get(),
              _detail::AmqpValueFactory::ToImplementation(
                  message.MessageAnnotations.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set message annotations.");
      }
    }

    if (!message.ApplicationProperties.empty())
    {
      AmqpMap appProperties;
      for (auto const& val : message.ApplicationProperties)
      {
        if ((val.second.GetType() == AmqpValueType::List)
            || (val.second.GetType() == AmqpValueType::Map)
            || (val.second.GetType() == AmqpValueType::Composite)
            || (val.second.GetType() == AmqpValueType::Described))
        {
          throw std::runtime_error(
              "Message Application Property values must be simple value types");
        }
        appProperties.emplace(val);
      }
      if (message_set_application_properties(
              rv.get(), _detail::AmqpValueFactory::ToImplementation(appProperties.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set application properties.");
      }
    }

    if (!message.DeliveryTag.IsNull())
    {
      if (message_set_delivery_tag(
              rv.get(), _detail::AmqpValueFactory::ToImplementation(message.DeliveryTag)))
      {
        throw std::runtime_error("Could not set delivery tag.");
      }
    }

    if (!message.Footer.empty())
    {
      if (message_set_footer(
              rv.get(), _detail::AmqpValueFactory::ToImplementation(message.Footer.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set message annotations.");
      }
    }
    switch (message.BodyType)
    {
      case MessageBodyType::None:
        break;
      case MessageBodyType::Data:
        for (auto const& binaryVal : message.m_binaryDataBody)
        {
          BINARY_DATA valueData{};
          valueData.bytes = binaryVal.data();
          valueData.length = static_cast<uint32_t>(binaryVal.size());
          if (message_add_body_amqp_data(rv.get(), valueData))
          {
            throw std::runtime_error("Could not set message body AMQP sequence value.");
          }
        }
        break;
      case MessageBodyType::Sequence:
        for (auto const& sequenceVal : message.m_amqpSequenceBody)
        {
          if (message_add_body_amqp_sequence(
                  rv.get(), _detail::AmqpValueFactory::ToImplementation(sequenceVal.AsAmqpValue())))
          {
            throw std::runtime_error("Could not set message body AMQP sequence value.");
          }
        }
        break;
      case MessageBodyType::Value:
        if (message_set_body_amqp_value(
                rv.get(), _detail::AmqpValueFactory::ToImplementation(message.m_amqpValueBody)))
        {
          throw std::runtime_error("Could not set message body AMQP value.");
        }
        break;
      case MessageBodyType::Invalid:
      default:
        throw std::runtime_error("Unknown message body type.");
    }
    return rv;
  }
#endif

  std::vector<AmqpList> const& AmqpMessage::GetBodyAsAmqpList() const
  {
    if (BodyType != MessageBodyType::Sequence)
    {
      throw std::runtime_error("Invalid body type, should be MessageBodyType::Sequence.");
    }
    return m_amqpSequenceBody;
  }

  void AmqpMessage::SetBody(AmqpBinaryData const& value)
  {
    BodyType = MessageBodyType::Data;
    m_binaryDataBody.push_back(value);
  }
  void AmqpMessage::SetBody(std::vector<AmqpBinaryData> const& value)
  {
    BodyType = MessageBodyType::Data;
    m_binaryDataBody = value;
  }
  void AmqpMessage::SetBody(AmqpValue const& value)
  {
    BodyType = MessageBodyType::Value;
    m_amqpValueBody = value;
  }
  void AmqpMessage::SetBody(std::vector<AmqpList> const& value)
  {
    BodyType = MessageBodyType::Sequence;
    m_amqpSequenceBody = value;
  }
  void AmqpMessage::SetBody(AmqpList const& value)
  {
    BodyType = MessageBodyType::Sequence;
    m_amqpSequenceBody.push_back(value);
  }

  AmqpValue const& AmqpMessage::GetBodyAsAmqpValue() const
  {
    if (BodyType != MessageBodyType::Value)
    {
      throw std::runtime_error("Invalid body type, should be MessageBodyType::Value.");
    }
    return m_amqpValueBody;
  }
  std::vector<AmqpBinaryData> const& AmqpMessage::GetBodyAsBinary() const
  {
    if (BodyType != MessageBodyType::Data)
    {
      throw std::runtime_error("Invalid body type, should be MessageBodyType::Value.");
    }
    return m_binaryDataBody;
  }

  bool AmqpMessage::operator==(AmqpMessage const& that) const noexcept
  {
    return (Header == that.Header) && (DeliveryAnnotations == that.DeliveryAnnotations)
        && (MessageAnnotations == that.MessageAnnotations) && (Properties == that.Properties)
        && (ApplicationProperties == that.ApplicationProperties) && (Footer == that.Footer)
        && (BodyType == that.BodyType) && (m_amqpValueBody == that.m_amqpValueBody)
        && (m_amqpSequenceBody == that.m_amqpSequenceBody)
        && (m_binaryDataBody == that.m_binaryDataBody);
  }

  std::vector<uint8_t> AmqpMessage::Serialize(AmqpMessage const& message)
  {
#if ENABLE_UAMQP
    std::vector<uint8_t> rv;

    // Append the message Header to the serialized message.
    if (message.Header.ShouldSerialize())
    {
      auto serializedHeader = MessageHeader::Serialize(message.Header);
      rv.insert(rv.end(), serializedHeader.begin(), serializedHeader.end());
    }
    if (!message.DeliveryAnnotations.empty())
    {
      AmqpValue deliveryAnnotations{
          _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{
              amqpvalue_create_delivery_annotations(_detail::AmqpValueFactory::ToImplementation(
                  message.DeliveryAnnotations.AsAmqpValue()))})};
      auto serializedDeliveryAnnotations = AmqpValue::Serialize(deliveryAnnotations);
      rv.insert(
          rv.end(), serializedDeliveryAnnotations.begin(), serializedDeliveryAnnotations.end());
    }
    if (!message.MessageAnnotations.empty())
    {
      AmqpValue messageAnnotations{
          _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{
              amqpvalue_create_message_annotations(_detail::AmqpValueFactory::ToImplementation(
                  message.MessageAnnotations.AsAmqpValue()))})};
      auto serializedAnnotations = AmqpValue::Serialize(messageAnnotations);
      rv.insert(rv.end(), serializedAnnotations.begin(), serializedAnnotations.end());
    }

    if (message.Properties.ShouldSerialize())
    {
      auto serializedMessageProperties = MessageProperties::Serialize(message.Properties);
      rv.insert(rv.end(), serializedMessageProperties.begin(), serializedMessageProperties.end());
    }

    if (!message.ApplicationProperties.empty())
    {
      AmqpMap appProperties;
      for (auto const& val : message.ApplicationProperties)
      {
        if ((val.second.GetType() == AmqpValueType::List)
            || (val.second.GetType() == AmqpValueType::Map)
            || (val.second.GetType() == AmqpValueType::Composite)
            || (val.second.GetType() == AmqpValueType::Described))
        {
          throw std::runtime_error(
              "Message Application Property values must be simple value types");
        }
        appProperties.emplace(val);
      }
      AmqpValue propertiesValue{Models::_detail::AmqpValueFactory::FromImplementation(
          Models::_detail::UniqueAmqpValueHandle{amqpvalue_create_application_properties(
              Models::_detail::AmqpValueFactory::ToImplementation(appProperties.AsAmqpValue()))})};
      auto serializedApplicationProperties = AmqpValue::Serialize(propertiesValue);
      rv.insert(
          rv.end(), serializedApplicationProperties.begin(), serializedApplicationProperties.end());
    }

    switch (message.BodyType)
    {
      default:
      case MessageBodyType::Invalid:
        throw std::runtime_error("Invalid message body type.");

      case MessageBodyType::Value: {
        // The message body element is an AMQP Described type, create one and serialize the
        // described body.
        AmqpDescribed describedBody(
            static_cast<std::uint64_t>(AmqpDescriptors::DataAmqpValue), message.m_amqpValueBody);
        auto serializedBodyValue = AmqpValue::Serialize(describedBody.AsAmqpValue());
        rv.insert(rv.end(), serializedBodyValue.begin(), serializedBodyValue.end());
      }
      break;
      case MessageBodyType::Data:
        for (auto const& val : message.m_binaryDataBody)
        {
          AmqpDescribed describedBody(
              static_cast<std::uint64_t>(AmqpDescriptors::DataBinary), val.AsAmqpValue());
          auto serializedBodyValue = AmqpValue::Serialize(describedBody.AsAmqpValue());
          rv.insert(rv.end(), serializedBodyValue.begin(), serializedBodyValue.end());
        }
        break;
      case MessageBodyType::Sequence: {
        for (auto const& val : message.m_amqpSequenceBody)
        {
          AmqpDescribed describedBody(
              static_cast<std::uint64_t>(AmqpDescriptors::DataAmqpSequence), val.AsAmqpValue());
          auto serializedBodyValue = AmqpValue::Serialize(describedBody.AsAmqpValue());
          rv.insert(rv.end(), serializedBodyValue.begin(), serializedBodyValue.end());
        }
      }
    }
    if (!message.Footer.empty())
    {
      AmqpValue footer{Models::_detail::AmqpValueFactory::FromImplementation(
          Models::_detail::UniqueAmqpValueHandle{amqpvalue_create_footer(
              Models::_detail::AmqpValueFactory::ToImplementation(message.Footer.AsAmqpValue()))})};
      auto serializedFooter = AmqpValue::Serialize(footer);
      rv.insert(rv.end(), serializedFooter.begin(), serializedFooter.end());
    }

    return rv;
#else
    (void)message;
    throw std::runtime_error("AMQP library is not enabled.");
#endif
  }

#if ENABLE_UAMQP
  namespace {
    class AmqpMessageDeserializer final {
    public:
      AmqpMessageDeserializer()
          : m_decoder{amqpvalue_decoder_create(OnAmqpMessageFieldDecodedFn, this)}
      {
      }

      AmqpMessage operator()(std::uint8_t const* data, size_t size) const
      {
        if (amqpvalue_decode_bytes(m_decoder.get(), data, size))
        {
          throw std::runtime_error("Could not decode object");
        }
        return m_decodedValue;
      }

    private:
      UniqueAmqpDecoderHandle m_decoder;
      AmqpMessage m_decodedValue;
      // The message fields, in their expected order.
      std::set<Amqp::_detail::AmqpDescriptors> m_expectedMessageFields{
          AmqpDescriptors::Header,
          AmqpDescriptors::DeliveryAnnotations,
          AmqpDescriptors::MessageAnnotations,
          AmqpDescriptors::Properties,
          AmqpDescriptors::ApplicationProperties,
          AmqpDescriptors::DataAmqpSequence,
          AmqpDescriptors::DataAmqpValue,
          AmqpDescriptors::DataBinary,
          AmqpDescriptors::Footer};

      // Invoked on each descriptor encountered while decrypting the message.
      static void OnAmqpMessageFieldDecodedFn(void* context, AMQP_VALUE value)
      {
        auto deserializer = static_cast<AmqpMessageDeserializer*>(context);

        deserializer->OnAmqpMessageFieldDecoded(
            Models::_detail::AmqpValueFactory::FromImplementation(
                _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)}));
      }

      // Invoked when a message field
      void OnAmqpMessageFieldDecoded(AmqpValue value)
      {
        if (value.GetType() != AmqpValueType::Described)
        {
          throw std::runtime_error("Decoded message field whose type is NOT described.");
        }
        AmqpDescribed describedType(value.AsDescribed());
        if (describedType.GetDescriptor().GetType() != AmqpValueType::Ulong)
        {
          throw std::runtime_error("Decoded message field MUST be a LONG type.");
        }

        AmqpDescriptors fieldDescriptor(
            static_cast<AmqpDescriptors>(static_cast<uint64_t>(describedType.GetDescriptor())));
        if (m_expectedMessageFields.find(fieldDescriptor) == m_expectedMessageFields.end())
        {
          throw std::runtime_error("Found message field is not in the set of expected fields.");
        }
        else
        {
          // Once we've seen a field, we can remove that field from the set of expected fields,
          // and also the fields which must come before it.
          //
          // The two exceptions are the DataBinary and DataAmqpSequence  fields, which can have
          // more than one instance.
          switch (fieldDescriptor)
          {
            case AmqpDescriptors::Header:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              break;
            case AmqpDescriptors::DeliveryAnnotations:
              // Once we've seen a DeliveryAnnotations, we no longer expect to see a Header or
              // another DeliveryAnnotations.
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              break;
            case AmqpDescriptors::MessageAnnotations:
              // Once we've seen a MessageAnnotations, we no longer expect to see a Header,
              // DeliveryAnnotations, or a MessageAnnotations.
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              break;
            case AmqpDescriptors::Properties:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::Properties);
              break;
            case AmqpDescriptors::ApplicationProperties:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::Properties);
              m_expectedMessageFields.erase(AmqpDescriptors::ApplicationProperties);
              break;
            case AmqpDescriptors::DataAmqpSequence:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::Properties);
              m_expectedMessageFields.erase(AmqpDescriptors::ApplicationProperties);
              // When we see an DataAmqpSequence, we no longer expect to see any other data type.
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpValue);
              m_expectedMessageFields.erase(AmqpDescriptors::DataBinary);
              break;
            case AmqpDescriptors::DataAmqpValue:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::Properties);
              m_expectedMessageFields.erase(AmqpDescriptors::ApplicationProperties);
              // When we see an DataAmqpValue, we no longer expect to see any other data type.
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpValue);
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpSequence);
              m_expectedMessageFields.erase(AmqpDescriptors::DataBinary);
              break;
            case AmqpDescriptors::DataBinary:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::Properties);
              m_expectedMessageFields.erase(AmqpDescriptors::ApplicationProperties);
              // When we see an DataBinary, we no longer expect to see any other data type.
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpValue);
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpSequence);
              break;
            case AmqpDescriptors::Footer:
              m_expectedMessageFields.erase(AmqpDescriptors::Header);
              m_expectedMessageFields.erase(AmqpDescriptors::DeliveryAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::MessageAnnotations);
              m_expectedMessageFields.erase(AmqpDescriptors::Properties);
              m_expectedMessageFields.erase(AmqpDescriptors::ApplicationProperties);
              // When we see an DataBinary, we no longer expect to see any other data type.
              m_expectedMessageFields.erase(AmqpDescriptors::DataBinary);
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpValue);
              m_expectedMessageFields.erase(AmqpDescriptors::DataAmqpSequence);
              m_expectedMessageFields.erase(AmqpDescriptors::Footer);
              break;
            default:
              throw std::runtime_error("Unknown message descriptor.");
          }
        }

        switch (fieldDescriptor)
        {
          case AmqpDescriptors::Header: {
            UniqueMessageHeaderHandle messageHeader;
            HEADER_HANDLE h;
            if (amqpvalue_get_header(_detail::AmqpValueFactory::ToImplementation(value), &h))
            {
              throw std::runtime_error("Could not convert field to header.");
            }
            messageHeader.reset(h);
            h = nullptr;
            m_decodedValue.Header
                = _detail::MessageHeaderFactory::FromImplementation(messageHeader);
            break;
          }
          case AmqpDescriptors::DeliveryAnnotations:
            m_decodedValue.DeliveryAnnotations = describedType.GetValue().AsMap();
            break;
          case AmqpDescriptors::MessageAnnotations:
            m_decodedValue.MessageAnnotations = describedType.GetValue().AsMap();
            break;
          case AmqpDescriptors::Properties: {
            UniquePropertiesHandle properties;
            PROPERTIES_HANDLE h;
            if (amqpvalue_get_properties(_detail::AmqpValueFactory::ToImplementation(value), &h))
            {
              throw std::runtime_error("Could not convert field to header.");
            }
            properties.reset(h);
            h = nullptr;
            m_decodedValue.Properties = _detail::MessagePropertiesFactory::FromUamqp(properties);
            break;
          }
          case AmqpDescriptors::ApplicationProperties: {
            auto propertyMap = describedType.GetValue().AsMap();
            for (auto const& val : propertyMap)
            {
              if (val.first.GetType() != AmqpValueType::String)
              {
                throw std::runtime_error("Key of applications properties must be a string.");
              }
              if ((val.second.GetType() == AmqpValueType::List)
                  || (val.second.GetType() == AmqpValueType::Map)
                  || (val.second.GetType() == AmqpValueType::Composite)
                  || (val.second.GetType() == AmqpValueType::Described))
              {
                throw std::runtime_error(
                    "Message Application Property values must be simple value types");
              }
              m_decodedValue.ApplicationProperties.emplace(
                  static_cast<std::string>(val.first), val.second);
            }
            break;
          }
          case AmqpDescriptors::DataAmqpValue:
            m_decodedValue.SetBody(describedType.GetValue());
            break;
          case AmqpDescriptors::DataAmqpSequence:
            m_decodedValue.SetBody(describedType.GetValue().AsList());
            break;
          case AmqpDescriptors::DataBinary:
            // Each call to SetBody will append the binary value to the vector of binary bodies.
            m_decodedValue.SetBody(describedType.GetValue().AsBinary());
            break;
          case AmqpDescriptors::Footer:
            m_decodedValue.Footer = describedType.GetValue().AsMap();
            break;
          default:
            throw std::runtime_error("Unknown message descriptor.");
        }
      }
    };
  } // namespace
#endif // ENABLE_UAMQP

  AmqpMessage AmqpMessage::Deserialize(std::uint8_t const* buffer, size_t size)
  {
#if ENABLE_UAMQP
    return AmqpMessageDeserializer{}(buffer, size);
#else
    (void)buffer;
    (void)size;
    throw std::runtime_error("AMQP library is not enabled.");
#endif
  }

  std::ostream& operator<<(std::ostream& os, AmqpMessage const& message)
  {
    os << "Message: <" << std::endl;

    if (message.MessageFormat != AmqpDefaultMessageFormatValue)
    {
      os << "    Message Format: " << message.MessageFormat << std::endl;
    }
    os << "    " << message.Header << std::endl;
    os << "    " << message.Properties;

    {
      if (!message.ApplicationProperties.empty())
      {
        os << std::endl << "    Application Properties: ";
        for (auto const& val : message.ApplicationProperties)
        {
          os << "{" << val.first << ", " << val.second << "}";
        }
      }
    }
    if (!message.DeliveryAnnotations.empty())
    {
      os << std::endl << "    Delivery Annotations: ";
      for (auto const& val : message.DeliveryAnnotations)
      {
        os << "{" << val.first << ", " << val.second << "}";
      }
    }
    if (!message.MessageAnnotations.empty())
    {
      os << std::endl << "    Message Annotations: ";
      for (auto const& val : message.MessageAnnotations)
      {
        os << "{" << val.first << ", " << val.second << "}";
      }
    }
    if (!message.DeliveryTag.IsNull())
    {
      os << ", deliveryTag=" << message.DeliveryTag;
    }
    if (!message.Footer.empty())
    {
      os << std::endl << "   Footer: ";
      for (auto const& val : message.Footer)
      {
        os << "{" << val.first << ", " << val.second << "}";
      }
    }

    os << std::endl << "    Body: [";
    switch (message.BodyType)
    {
      case MessageBodyType::Invalid:
        os << "Invalid";
        break;
      case MessageBodyType::None:
        os << "None";
        break;
      case MessageBodyType::Data: {
        os << "AmqpBinaryData: [";
        auto const& bodyBinary = message.GetBodyAsBinary();
        uint8_t i = 0;
        for (auto const& val : bodyBinary)
        {
          os << val.size() << " bytes";
          if (i < bodyBinary.size() - 1)
          {
            os << ", ";
          }
          i += 1;
        }
        os << "]";
      }
      break;
      case MessageBodyType::Sequence: {
        os << "AmqpSequence: [";
        auto const& bodySequence = message.GetBodyAsAmqpList();
        uint8_t i = 0;
        for (auto const& val : bodySequence)
        {
          os << "{Sequence: ";
          uint8_t j = 0;
          for (auto const& seqVal : val)
          {
            os << seqVal.GetType();
            if (j < val.size() - 1)
            {
              os << ", ";
            }
            j += 1;
          }
          os << "}";
          if (i < bodySequence.size() - 1)
          {
            os << ", ";
          }
          i += 1;
        }
        os << "]";
      }
      break;
      case MessageBodyType::Value:
        os << "AmqpValue, type=" << message.GetBodyAsAmqpValue().GetType();
        break;
    }
    os << std::endl << ">";

    return os;
  }
}}}} // namespace Azure::Core::Amqp::Models
