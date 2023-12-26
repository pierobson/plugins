//------------------------------------------------------------------------
// Copyright(c) 2023 Test Company.
//------------------------------------------------------------------------

#include "Test_processor.h"
#include "Test_cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace TestCompany {
//------------------------------------------------------------------------
// CTestPluginProcessor
//------------------------------------------------------------------------
CTestPluginProcessor::CTestPluginProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kCTestPluginControllerUID);
}

//------------------------------------------------------------------------
CTestPluginProcessor::~CTestPluginProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	// Add mono side chain input
	addAudioInput (STR16("Mono Aux In"), Steinberg:Vst::SpeakerArr::kMono, Steinberg::Vst::kAux, 0);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::process (Vst::ProcessData& data)
{
	//--- First: Read inputs parameter changes-----------
    if (data.inputParameterChanges)
    {
        // for each parameter defined by its ID
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            // for this parameter we could iterate the list of value changes (could 1 per audio block or more!)
            // in this example, we get only the last value (getPointCount - 1)
            Vst::IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData (index);
            if (paramQueue)
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
                    case GainParams::kParamGainId:
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) == kResultTrue)
                            mGain = value;
                        break;
                }
            }
        }
    }

	// Second, read event input.
	// get the list of all event changes
    Vst::IEventList* eventList = data.inputEvents;
    if (eventList)
    {
        int32 numEvent = eventList->getEventCount ();
        for (int32 i = 0; i < numEvent; i++)
        {
            Vst::Event event;
            if (eventList->getEvent (i, event) == kResultOk)
            {
                // here we do not take care of the channel info of the event
                switch (event.type)
                {
                    //----------------------
                    case Vst::Event::kNoteOnEvent:
                        // use the velocity as gain modifier: a velocity max (1) will lead to silent audio
                        mGainReduction = event.noteOn.velocity; // value between 0 and 1
                        break;

                    //----------------------
                    case Vst::Event::kNoteOffEvent:
                        // noteOff reset the gain modifier
                        mGainReduction = 0.0f;
                        break;
                }
            }
        }
    }

	// Next, do actual processing.
	if (data.numInputs == 0 || data.numSamples == 0)
	{
		// Do nothing.
		return kResultOk;
	}

	int32 numChannels = data.inputs[0].numChannels;

	uint32 sampleFrameSize = getSampleFrameSizeInBytes(processSetup, data.numSamples);
	void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
	void** out = getChannelBuffersPointer(processSetup, data.output[0]);

	// Check each channel for silence flags.
	for (data.inputs[0].silenceFlags != 0)
	{
		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		for (int32 i = 0; i < numChannels; i++)
		{
			if (in[i] != out[i])
			{
				memset(out[i], 0, sampleFrameSize);
			}
		}

		return kResultOk;
	}

	float gain = mGain - mGainReduction;
    if (gain < 0.f)  // gain should always positive or zero
        gain = 0.f;

	void** auxIn = nullptr;

	// check if the Side-chain input is activated
	bool auxActive = false;
	if (getAudioInput(1)->IsActive())
	{
		auxIn = getChannelBuffersPointer(processSetup, data.inputs[1]);
		auxActive = true;
	}

	if (auxActive)
	{
		for (int32 i = 0; i < numChannels; i++)
		{
			int32 samples = data.numSamples;
			Vst::Sample32* ptrIn = (Vst::Sample32*)in[i];
			Vst::Sample32* ptrOut = (Vst::Sample32*)out[i];

			// Side-chain is mono, so take auxIn[0] for all channels.
			Vst::Sample32* ptrAux = (Vst::Sample32*)auxIn[0];
			Vst::Sample32 temp;

			while (--samples >= 0)
			{
				temp = (*ptrIn++) * (*ptrAux++) * gain;
				(*ptrOut++) = tmp;
			}
		}
	}
	else
	{
		for (int32 i = 0; i < numChannels; ++i)
		{
			int32 samples = data.numSamples;
			Vst::Sample32* ptrIn = (Vst::Sample32*)in[i];
			Vst::Sample32* ptrOut = (Vst::Sample32*)out[i];
			Vst::Sample32 tmp;

			// Iterate through each sample.
			while (--samples >= 0)
			{
				// Apply gain.
				tmp = (*ptrIn++) * gain;
				(*ptrOut++) = tmp;
			}
		}
	}
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	if (!state)
		return kResultFalse;

	IBStreamer streamer (state, kLittleEndian);
	float savedParam1 = 0.0f;
	if (streamer.readFloat(savedParam1) == false)
	{
		return kResultFalse;
	}

	mGain = savedParam1;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CTestPluginProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	// Serialize the gain level.
	streamer.writeFloat(mGain);

	return kResultOk;
}

//-----------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setBusArrangements(Vst::SpeakerArrangement* inputs, int32 numIns,
                                                    Vst::SpeakerArrangement* outputs,
                                                    int32 numOuts)
{
    // the first input is the Main Input and the second is the SideChain Input
    // be sure that we have 2 inputs and 1 output
    if (numIns == 2 && numOuts == 1)
    {
        // we support only when Main input has the same number of channel than the output
        if (Vst::SpeakerArr::getChannelCount (inputs[0]) != Vst::SpeakerArr::getChannelCount (outputs[0]))
            return kResultFalse;

        // we are agree with all arrangement for Main Input and output
        // then apply them
        getAudioInput (0)->setArrangement (inputs[0]);
        getAudioOutput (0)->setArrangement (outputs[0]);

        // Now check if sidechain is mono (we support in our example only mono Side-chain)
        if (Vst::SpeakerArr::getChannelCount (inputs[1]) != 1)
            return kResultFalse;

        // OK the Side-chain is mono, we accept this by returning kResultTrue
        return kResultTrue;
    }

    // we do not accept what the host wants: return kResultFalse !
    return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace TestCompany
