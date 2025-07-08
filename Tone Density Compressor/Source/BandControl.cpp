/*
  ==============================================================================

    BandControl.cpp
    Created: 4 Jul 2025 11:15:23am
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "BandControl.h"

BandControl::BandControl(const juce::String& name) : bandName(name)
{
    addAndMakeVisible(sensitivityKnob);
    addAndMakeVisible(compressionKnob);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(bypassLabel);

    bypassLabel.setJustificationType(juce::Justification::centred);
    bypassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
}

void BandControl::attachParameters(juce::AudioProcessorValueTreeState& params, const juce::String& prefix)
{
    sensitivityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "Sensitivity", sensitivityKnob.getSlider());

    compressionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "Compression", compressionKnob.getSlider());

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, prefix + "Bypass", bypassButton);
}

void BandControl::paint(juce::Graphics& g)
{
    // Optional: Draw a background or outlines here if desired
    // Example:
    // g.setColour(juce::Colours::darkgrey);
    // g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(5.0f), 8.0f, 2.0f);
}

void BandControl::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto knobWidth = area.getWidth() / 3;

    // Sensitivity knob
    auto sensArea = area.removeFromLeft(knobWidth);
    sensitivityKnob.setBounds(sensArea);

    // Compression knob
    auto compArea = area.removeFromLeft(knobWidth);
    compressionKnob.setBounds(compArea);

    // Bypass button & label
    auto bypassArea = area;
    bypassButton.setBounds(bypassArea.removeFromTop(30)); // Adjust height as needed
    bypassLabel.setBounds(bypassArea);
}
