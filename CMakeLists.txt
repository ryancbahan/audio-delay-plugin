cmake_minimum_required(VERSION 3.15)

project(AudioDelay VERSION 1.0.0)

# Include FetchContent module
include(FetchContent)

# Fetch JUCE
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.5
)
FetchContent_MakeAvailable(JUCE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Create the plugin target
juce_add_plugin(AudioDelay
    COMPANY_NAME "bahan audio"
    PLUGIN_MANUFACTURER_CODE Ymnf
    PLUGIN_CODE Adly
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "AudioDelay"
)

# Add source files
target_sources(AudioDelay
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/LFOManager.cpp
        Source/DelayManager.cpp
)

# Add JUCE modules
target_compile_definitions(AudioDelay
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link to JUCE modules
target_link_libraries(AudioDelay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Define the paths
set(AU_COMPONENT_PATH "${CMAKE_BINARY_DIR}/AudioDelay_artefacts/Debug/AU/AudioDelay.component")
set(AU_DESTINATION_PATH "/Library/Audio/Plug-Ins/Components/AudioDelay.component")

# Add custom command to copy the AU component
add_custom_command(
    TARGET AudioDelay
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying AU component to Audio Units directory..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${AU_COMPONENT_PATH}"
        "${AU_DESTINATION_PATH}"
    COMMAND ${CMAKE_COMMAND} -E echo "AU component copied successfully to ${AU_DESTINATION_PATH}"
)