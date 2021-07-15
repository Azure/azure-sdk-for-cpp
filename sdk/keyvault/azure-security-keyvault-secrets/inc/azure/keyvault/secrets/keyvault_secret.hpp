/*"SecretBundle" : {
  "properties" : {
    "value" : {"type" : "string", "description" : "The secret value."},
    "id" : {"type" : "string", "description" : "The secret id."},
    "contentType" : {"type" : "string", "description" : "The content type of the secret."},
    "attributes" : {
      "$ref" : "#/definitions/SecretAttributes",
      "description" : "The secret management attributes."
    },
    "tags" : {
      "type" : "object",
      "additionalProperties" : {"type" : "string"},
      "description" : "Application specific metadata in the form of key-value pairs."
    },
    "kid" : {
      "type" : "string",
      "readOnly" : true,
      "description" : "If this is a secret backing a KV certificate, then this field specifies the "
                      "corresponding key backing the KV certificate."
    },
    "managed" : {
      "type" : "boolean",
      "readOnly" : true,
      "description" : "True if the secret's lifetime is managed by key vault. If this is a secret "
                      "backing a certificate, then managed will be true."
    }
  },
  "description" : "A secret consisting of a value, id and its attributes."
},
*/
struct KeyVaultSecret final
{
  /**
  * @brief The secret value.
  * 
  */
  std::string Value;

  /**
  * @brief The secret id.
  * 
  */
  std::string Id;

  /**
   * @brief The content type of the secret.
   *
   */
  std::string ContentType;

  /**
   * @brief  If this is a secret backing a KV certificate, then this field specifies the 
   * corresponding key backing the KV certificate.
   *
   */
  std::string KeyId;

  /**
   * @brief Application specific metadata in the form of key-value pairs.
   *
   */
  std::unordered_map<std::string, std::string> Tags;

  /**
   * @brief True if the secret's lifetime is managed by key vault. If this is a secret 
   * backing a certificate, then managed will be true.
   *
   */
  bool Managed;

  /**
   * @brief Construct a new SecretBundle object.
   *
   */
  KeyVaultSecret() = default;
};
