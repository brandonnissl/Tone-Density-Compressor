#pragma once

#include <JuceHeader.h>
#include "DotKnob.h"  // Include your new reusable DotKnob component here

class BandControl : public juce::Component
{
public:
    BandControl(const juce::String& name);
    ~BandControl() override = default;

    void attachParameters(juce::AudioProcessorValueTreeState& params, const juce::String& prefix);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DotKnob sensitivityKnob { "Sensitivity" };   // Now using DotKnob instead of Slider
    DotKnob compressionKnob { "Compression" };   // Same here
    juce::ToggleButton bypassButton;
    
    juce::Label bypassLabel { "", "Bypass" };  // Keeping bypass label only, knobs already have labels inside

    juce::String bandName;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sensitivityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compressionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandControl)
};
