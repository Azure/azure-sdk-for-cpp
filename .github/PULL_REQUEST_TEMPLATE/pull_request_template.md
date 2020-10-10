# Pull Request Checklist

Please leverage this checklist as a reminder to address commonly occurring feedback when submitting a pull request to make sure your PR can be reviewed quickly:

- [ ] Does the API design and implementation within your PR follow the [C++ Guidelines](https://azure.github.io/azure-sdk/cpp_introduction.html)?
  - [ ] Do you have an issue or spec doc that describes the design, usage, and motivating scenario?
- [ ] If you are modifying header files, are the changes documented using doxygen style comments?
- [ ] Have you added relevant unit tests to ensure CI will catch future regressions?
- [ ] Have you reviewed your own PR to make sure there aren't any unintended changes or commits?
- [ ] Does your PR have a descriptive title and description (with issue number), which explains why the change is being made?
  - [ ] Does your PR have a single purpose or does it have other unrelated changes that can be separated out into its own PR?
- [ ] Do you have any complex components that could use comments in source to explain (focusing on the "why")?
- [ ] Are there any typos or spelling errors, especially in user-facing documentation?
- [ ] Is the PR actually ready for review or is it work-in-progress (WIP) or an experiment? If so, can it start off as a draft PR?
- [ ] Do your changes have impact elsewhere? For instance, do you need to update other docs or exiting markdown files that might be impacted?
- [ ] Does the PR contain any breaking changes?
