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
    addAndMakeVisible(sensitivityLabel);
    addAndMakeVisible(compressionLabel);
    addAndMakeVisible(bypassLabel);

    sensitivityLabel.setJustificationType(juce::Justification::centred);
    compressionLabel.setJustificationType(juce::Justification::centred);
    bypassLabel.setJustificationType(juce::Justification::centred);
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

    auto sensArea = area.removeFromLeft(knobWidth);
    sensitivityKnob.setBounds(sensArea.removeFromTop(knobWidth));
    sensitivityLabel.setBounds(sensArea);

    auto compArea = area.removeFromLeft(knobWidth);
    compressionKnob.setBounds(compArea.removeFromTop(knobWidth));
    compressionLabel.setBounds(compArea);

    auto bypassArea = area;
    bypassButton.setBounds(bypassArea.removeFromTop(knobWidth));
    bypassLabel.setBounds(bypassArea);
}
