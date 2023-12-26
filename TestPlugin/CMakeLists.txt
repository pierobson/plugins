cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "../../vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(TestPlugin
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "TestPlugin VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(TestPlugin
    source/version.h
    source/Test_cids.h
    source/Test_processor.h
    source/Test_processor.cpp
    source/Test_controller.h
    source/Test_controller.cpp
    source/Test_entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(TestPlugin
        PRIVATE
            resource/Test_editor.uidesc
    )
    target_link_libraries(TestPlugin
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(TestPlugin
        RESOURCES
            "resource/Test_editor.uidesc"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (TestPlugin
    RESOURCES
        resource/97F7431DECCC5CBAB8BF4C3693F98ABC_snapshot.png
        resource/97F7431DECCC5CBAB8BF4C3693F98ABC_snapshot_2.0x.png
)

target_link_libraries(TestPlugin
    PRIVATE
        sdk
)

smtg_target_configure_version_file(TestPlugin)

if(SMTG_MAC)
    smtg_target_set_bundle(TestPlugin
        BUNDLE_IDENTIFIER com.testcompany.vst3.testplugin
        COMPANY_NAME "Test Company"
    )
    smtg_target_set_debug_executable(TestPlugin
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(TestPlugin PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestPlugin)

        smtg_target_set_debug_executable(TestPlugin
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
