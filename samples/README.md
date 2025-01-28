# Samples, Snippets, and How-To Guides

Developers like to learn by looking at code, and so the Azure SDK comes with a myriad of code samples in the form of short code snippets, sample applications, and how-to guides. This document describes where to find all these resources.

## Structure of the Repository

The Azure SDK repository is organized in the following folder structure, with the main sample locations highlighted using **bold** font.

`/samples` (this folder)<br>
&nbsp;&nbsp;&nbsp;&nbsp;`README.md` (this file)<br>
`/sdk` (folder containing sources, samples, test for all SDK packages)<br>
&nbsp;&nbsp;&nbsp;&nbsp;`/<service>` (e.g. storage)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`/<package>` (e.g. blobs)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**`README.md`** (package READMEs contain hello world samples)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**`/samples`** (package-specific samples)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`/inc` (header files)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`/src` (implementation)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`/test`<br>

##  Getting Started (a.k.a. `Hello World`) Samples

Each package folder contains a package-specific `README.md` file. Most of these `README` files contain `Hello World` code samples illustrating basic usage of the the APIs contained in the package. For example, you can find `Hello World` samples for the `azure-storage-blobs` package [here](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/README.md#code-samples).

## Package Samples and How-To Guides

Each package folder contains a subfolder called `/samples` with additional code samples. These samples can be either short programs contained in `*.c` files, or more complete how-to guides (code samples and some commentary) contained in `*.md` files. You can find shortcuts to main how-to guides in the [**How-To Guides List**](#how-to-guide-list) section below.

## Integration Samples

Simple applications that illustrate the different approaches to integrate the Azure SDK for C++ to your application or library. Each sample contains a README with a description and explanation.

## Sample Applications

Sometimes we want to illustrate how several APIs or even packages work together in a context of a more complete program. For these cases, we created sample applications that you can look at, download, compile, and execute. These application samples are located on
[https://learn.microsoft.com/samples/](https://learn.microsoft.com/samples/).

## How-To Guide List

This section lists how-to guides for the most commonly used APIs and most common scenarios, i.e. this section does not attempt to be a complete directory of guides contained in this repository.

#### General How-To Guides

- How to configure, access, and analyze **logging** information (TODO)

## Contributing

This project welcomes contributions and suggestions. Most contributions require you to agree to a [Contributor License Agreement (CLA)][cla] declaring that you have the right to, and actually do, grant us the rights to use your contribution.

If you'd like to contribute to this library, please read the [contributing guide][contributing_guide] to learn more about how to build and test the code.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct][coc]. For more information, see the [Code of Conduct FAQ][coc_faq] or contact [opencode@microsoft.com][coc_contact] with any additional questions or comments.

<!-- LINKS -->
[contributing_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[cla]: https://cla.microsoft.com
[coc]: https://opensource.microsoft.com/codeofconduct/
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[coc_contact]: mailto:opencode@microsoft.com

