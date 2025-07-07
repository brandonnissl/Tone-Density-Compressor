/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "foleys_gui_magic/foleys_gui_magic.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ToneDensityCompressorAudioProcessorEditor  : public foleys::MagicPluginEditor
{
public:
    ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor&);
    ~ToneDensityCompressorAudioProcessorEditor() override = default;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneDensityCompressorAudioProcessorEditor)
};
