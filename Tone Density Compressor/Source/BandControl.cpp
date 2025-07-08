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
    addAndMakeVisible(freqLowSlider);
    addAndMakeVisible(freqHighSlider);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(bypassLabel);
    addAndMakeVisible(lowLabel);
    addAndMakeVisible(highLabel);

    bypassLabel.setJustificationType(juce::Justification::centred);
    bypassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    lowLabel.setJustificationType(juce::Justification::centred);
    highLabel.setJustificationType(juce::Justification::centred);
    lowLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    highLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    freqLowSlider.setSliderStyle(juce::Slider::LinearVertical);
    freqHighSlider.setSliderStyle(juce::Slider::LinearVertical);
    freqLowSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    freqHighSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

void BandControl::attachParameters(juce::AudioProcessorValueTreeState& params, const juce::String& prefix)
{
    sensitivityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "Sensitivity", sensitivityKnob.getSlider());

    compressionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "Compression", compressionKnob.getSlider());

    freqLowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "FreqLow", freqLowSlider);
    freqHighAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, prefix + "FreqHigh", freqHighSlider);

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
    auto itemWidth = area.getWidth() / 5;

    auto sensArea = area.removeFromLeft(itemWidth);
    sensitivityKnob.setBounds(sensArea);

    auto compArea = area.removeFromLeft(itemWidth);
    compressionKnob.setBounds(compArea);

    auto lowArea = area.removeFromLeft(itemWidth);
    freqLowSlider.setBounds(lowArea.removeFromTop(area.getHeight() - 20));
    lowLabel.setBounds(lowArea);

    auto highArea = area.removeFromLeft(itemWidth);
    freqHighSlider.setBounds(highArea.removeFromTop(area.getHeight() - 20));
    highLabel.setBounds(highArea);

    auto bypassArea = area;
    bypassButton.setBounds(bypassArea.removeFromTop(30));
    bypassLabel.setBounds(bypassArea);
}
