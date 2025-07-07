#pragma once

#include <JuceHeader.h>
#include "LookAndFeel_TDC.h"

class GlobalControlComponent : public juce::Component
{
public:
    GlobalControlComponent();
    ~GlobalControlComponent() override;

    void attachParameters(juce::AudioProcessorValueTreeState& params);

    void resized() override;

private:
    LookAndFeel_TDC customLookAndFeel;

    juce::Slider mixKnob;
    juce::Slider outputKnob;
    juce::ToggleButton linkButton;
    juce::ToggleButton bypassButton;
    juce::Label mixLabel {"", "Mix"};
    juce::Label outputLabel {"", "Output"};
    juce::Label linkLabel {"", "Link"};
    juce::Label bypassLabel {"", "Bypass"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalControlComponent)
};
