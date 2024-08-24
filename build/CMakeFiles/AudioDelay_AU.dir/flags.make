# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# compile CXX with /Library/Developer/CommandLineTools/usr/bin/c++
CXX_DEFINES = -DAudioDelay_AU_EXPORTS -DDEBUG=1 -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_MODULE_AVAILABLE_juce_audio_basics=1 -DJUCE_MODULE_AVAILABLE_juce_audio_devices=1 -DJUCE_MODULE_AVAILABLE_juce_audio_formats=1 -DJUCE_MODULE_AVAILABLE_juce_audio_processors=1 -DJUCE_MODULE_AVAILABLE_juce_audio_utils=1 -DJUCE_MODULE_AVAILABLE_juce_core=1 -DJUCE_MODULE_AVAILABLE_juce_data_structures=1 -DJUCE_MODULE_AVAILABLE_juce_events=1 -DJUCE_MODULE_AVAILABLE_juce_graphics=1 -DJUCE_MODULE_AVAILABLE_juce_gui_basics=1 -DJUCE_MODULE_AVAILABLE_juce_gui_extra=1 -DJUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone -DJUCE_USE_CURL=0 -DJUCE_VST3_CAN_REPLACE_VST2=0 -DJUCE_WEB_BROWSER=0 -DJucePlugin_AAXCategory=0 -DJucePlugin_AAXDisableBypass=0 -DJucePlugin_AAXDisableMultiMono=0 -DJucePlugin_AAXIdentifier="com.bahan audio.AudioDelay" -DJucePlugin_AAXManufacturerCode=JucePlugin_ManufacturerCode -DJucePlugin_AAXProductId=JucePlugin_PluginCode -DJucePlugin_ARACompatibleArchiveIDs=\"\" -DJucePlugin_ARAContentTypes=0 -DJucePlugin_ARADocumentArchiveID="\"com.bahan audio.AudioDelay.aradocumentarchive.1\"" -DJucePlugin_ARAFactoryID="\"com.bahan audio.AudioDelay.arafactory.1.0.0\"" -DJucePlugin_ARATransformationFlags=0 -DJucePlugin_AUExportPrefix=AudioDelayAU -DJucePlugin_AUExportPrefixQuoted=\"AudioDelayAU\" -DJucePlugin_AUMainType="'aufx'" -DJucePlugin_AUManufacturerCode=JucePlugin_ManufacturerCode -DJucePlugin_AUSubType=JucePlugin_PluginCode -DJucePlugin_Build_AAX=0 -DJucePlugin_Build_AU=1 -DJucePlugin_Build_AUv3=0 -DJucePlugin_Build_LV2=0 -DJucePlugin_Build_Standalone=0 -DJucePlugin_Build_Unity=0 -DJucePlugin_Build_VST3=0 -DJucePlugin_Build_VST=0 -DJucePlugin_CFBundleIdentifier="com.bahan audio.AudioDelay" -DJucePlugin_Desc=\"AudioDelay\" -DJucePlugin_EditorRequiresKeyboardFocus=0 -DJucePlugin_Enable_ARA=0 -DJucePlugin_IsMidiEffect=0 -DJucePlugin_IsSynth=0 -DJucePlugin_Manufacturer="\"bahan audio\"" -DJucePlugin_ManufacturerCode=0x596d6e66 -DJucePlugin_ManufacturerEmail=\"\" -DJucePlugin_ManufacturerWebsite=\"\" -DJucePlugin_Name=\"AudioDelay\" -DJucePlugin_PluginCode=0x41646c79 -DJucePlugin_ProducesMidiOutput=0 -DJucePlugin_VSTCategory=kPlugCategEffect -DJucePlugin_VSTNumMidiInputs=16 -DJucePlugin_VSTNumMidiOutputs=16 -DJucePlugin_VSTUniqueID=JucePlugin_PluginCode -DJucePlugin_Version=1.0.0 -DJucePlugin_VersionCode=0x10000 -DJucePlugin_VersionString=\"1.0.0\" -DJucePlugin_Vst3Category=\"Fx\" -DJucePlugin_WantsMidiInput=0 -D_DEBUG=1

CXX_INCLUDES = -I/Users/ryanbahan/audiodelay/build/AudioDelay_artefacts/JuceLibraryCode -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/VST3_SDK -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/lv2 -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/serd -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/sord -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/sord/src -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/sratom -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/lilv -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_processors/format_types/LV2_SDK/lilv/src -I/Users/ryanbahan/audiodelay/build/_deps/juce-src/modules/juce_audio_plugin_client/AU

CXX_FLAGSarm64 = -g -std=gnu++17 -arch arm64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX14.4.sdk -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -g -O0 -Wall -Wshadow-all -Wshorten-64-to-32 -Wstrict-aliasing -Wuninitialized -Wunused-parameter -Wconversion -Wsign-compare -Wint-conversion -Wconditional-uninitialized -Wconstant-conversion -Wsign-conversion -Wbool-conversion -Wextra-semi -Wunreachable-code -Wcast-align -Wshift-sign-overflow -Wmissing-prototypes -Wnullable-to-nonnull-conversion -Wno-ignored-qualifiers -Wswitch-enum -Wpedantic -Wdeprecated -Wzero-as-null-pointer-constant -Wunused-private-field -Woverloaded-virtual -Wreorder -Winconsistent-missing-destructor-override

CXX_FLAGS = -g -std=gnu++17 -arch arm64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX14.4.sdk -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -g -O0 -Wall -Wshadow-all -Wshorten-64-to-32 -Wstrict-aliasing -Wuninitialized -Wunused-parameter -Wconversion -Wsign-compare -Wint-conversion -Wconditional-uninitialized -Wconstant-conversion -Wsign-conversion -Wbool-conversion -Wextra-semi -Wunreachable-code -Wcast-align -Wshift-sign-overflow -Wmissing-prototypes -Wnullable-to-nonnull-conversion -Wno-ignored-qualifiers -Wswitch-enum -Wpedantic -Wdeprecated -Wzero-as-null-pointer-constant -Wunused-private-field -Woverloaded-virtual -Wreorder -Winconsistent-missing-destructor-override

