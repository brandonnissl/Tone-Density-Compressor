/*
  ==============================================================================

    HeaderComponent.h
    Created: 4 Jul 2025 11:20:06am
    Author:  Brandon Nissl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class HeaderComponent : public juce::Component
{
public:
    HeaderComponent() {}

    void paint (juce::Graphics& g) override {}
    void resized() override {}

private:
    juce::Label logoLabel;
    juce::ComboBox presetBox;
    juce::TextButton settingsButton {"⚙"};
};
