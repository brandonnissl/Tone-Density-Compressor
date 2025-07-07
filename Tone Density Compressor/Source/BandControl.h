#pragma once

#include <JuceHeader.h>

class BandControl : public juce::Component
{
public:
    BandControl(const juce::String& name);
    ~BandControl() override = default;

    void attachParameters(juce::AudioProcessorValueTreeState& params, const juce::String& prefix);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider sensitivityKnob;
    juce::Slider compressionKnob;
    juce::ToggleButton bypassButton;
    juce::Label sensitivityLabel {"", "Sensitivity"};
    juce::Label compressionLabel {"", "Compression"};
    juce::Label bypassLabel {"", "Bypass"};

    juce::String bandName;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sensitivityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compressionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandControl)
};
