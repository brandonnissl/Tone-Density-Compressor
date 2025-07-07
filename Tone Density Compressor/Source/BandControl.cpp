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
}

void BandControl::attachParameters(juce::AudioProcessorValueTreeState& params, const juce::String& prefix)
{
    sensitivityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "Sensitivity", sensitivityKnob);
    compressionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "Compression", compressionKnob);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, prefix + "Bypass", bypassButton);
}

void BandControl::paint(juce::Graphics& g)
{
    // Optional: Draw background or text here
}

void BandControl::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto knobWidth = area.getWidth() / 3;

    sensitivityKnob.setBounds(area.removeFromLeft(knobWidth));
    compressionKnob.setBounds(area.removeFromLeft(knobWidth));
    bypassButton.setBounds(area);
}
