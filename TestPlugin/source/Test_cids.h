//------------------------------------------------------------------------
// Copyright(c) 2023 Test Company.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace TestCompany {
//------------------------------------------------------------------------
static const Steinberg::FUID kCTestPluginProcessorUID (0x97F7431D, 0xECCC5CBA, 0xB8BF4C36, 0x93F98ABC);
static const Steinberg::FUID kCTestPluginControllerUID (0x9DE36595, 0xE9B759FE, 0xB2D59C36, 0xB9D0FF9C);

#define CTestPluginVST3Category "TestCategory"

enum GainParams : Steinberg::Vst::ParamID
{
    kParamGainId = 102,
};

//------------------------------------------------------------------------
} // namespace TestCompany
