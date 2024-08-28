#include <azure/identity.hpp> 
#include <azure/keyvault/secrets.hpp> 
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;

int main(){
    std::cout<<"Starting Program!"<<std::endl;

    try{
        // Set Key Vault URL string
        auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");

        // Create Defautl Azure Credential to Authenticate. I will pick up on our AzureCLI login
        auto credential = std::make_shared<Azure::Identity::AzureCliCredential>();

        // Create Key Vault Secret Client
        SecretClient secretClient(keyVaultUrl, credential);

        // Create a Secret
        std::string secretName("MySampleSecret");
        std::string secretValue("My super secret value");
        secretClient.SetSecret(secretName, secretValue);

        // Get the Secret
        KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;
        std::string valueString = secret.Value.HasValue()? secret.Value.Value() : "NONE RETURNED";
        std::cout<<"Secret is returned with name "<<secret.Name<<" and value "<<valueString<<std::endl;

    }catch (const Azure::Core::RequestFailedException& ex){
        std::cout << std::underlying_type<Azure::Core::Http::HttpStatusCode>::type(ex.StatusCode)<<std::endl;
    }

    std::cout<<"End of Program!"<<std::endl;
}