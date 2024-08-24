## Development

you can try using xcode, but it is a nightmare. vscode is suggested workflow:

- download JUCE https://juce.com/
- download c/c++ and cmake extensions for vscode
- run cmake configure (cmd+shift+p -> cmake configure)
- run cmake build (cmd+shift+p -> cmake buildd)

still very WIP, you may have to fiddle with the CMakeLists file to adjust for your machine (mainly plugin location paths)
