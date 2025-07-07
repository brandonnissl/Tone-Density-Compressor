/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HeaderComponent.h"
#include "PluginProcessor.h"
#include "SpectralFlatnessGraphComponent.h"
#include "BandControlComponent.h"
#include "GlobalControlComponent.h"
#include "IOAndDensityMeterComponent.h"
#include "LookAndFeel_TDC.h"

//==============================================================================
/**
*/
class ToneDensityCompressorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor&);
    ~ToneDensityCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

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
