/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <foleys_gui_magic/foleys_gui_magic.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

class ToneDensityCompressorAudioProcessorEditor  : public foleys::MagicPluginEditor

{
public:
    ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor&);
    ~ToneDensityCompressorAudioProcessorEditor() override = default;

    void timerCallback() override;

    juce::AudioBuffer<float> analysisBuffer;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ToneDensityCompressorAudioProcessor& audioProcessor;
    LookAndFeel_TDC customLookAndFeel;
    HeaderComponent header;
    SpectralFlatnessGraphComponent spectralGraph;
    BandControlComponent bandsControl;
    GlobalControlComponent globalControl;
    IOAndDensityMeterComponent ioMeters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneDensityCompressorAudioProcessorEditor)
};
