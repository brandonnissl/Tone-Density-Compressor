/*
  ==============================================================================

    GlobalControlComponent.cpp
    Created: 4 Jul 2025 11:15:33am
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "GlobalControlComponent.h"

GlobalControlComponent::GlobalControlComponent()
{
    mixKnob.setLookAndFeel(&customLookAndFeel);
    outputKnob.setLookAndFeel(&customLookAndFeel);
    linkButton.setLookAndFeel(&customLookAndFeel);
    bypassButton.setLookAndFeel(&customLookAndFeel);
    autoGainButton.setLookAndFeel(&customLookAndFeel);
    oversamplingBox.setLookAndFeel(&customLookAndFeel);
    midSideButton.setLookAndFeel(&customLookAndFeel);


    addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputKnob);
    addAndMakeVisible(linkButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(autoGainButton);
    addAndMakeVisible(oversamplingBox);
    addAndMakeVisible(midSideButton);

    addAndMakeVisible(mixLabel);
    addAndMakeVisible(outputLabel);
    addAndMakeVisible(linkLabel);
    addAndMakeVisible(bypassLabel);
    addAndMakeVisible(autoGainLabel);
    addAndMakeVisible(oversamplingLabel);
    addAndMakeVisible(midSideLabel);


    mixLabel.setJustificationType(juce::Justification::centred);
    outputLabel.setJustificationType(juce::Justification::centred);
    linkLabel.setJustificationType(juce::Justification::centred);
    bypassLabel.setJustificationType(juce::Justification::centred);
    autoGainLabel.setJustificationType(juce::Justification::centred);
    oversamplingLabel.setJustificationType(juce::Justification::centred);
    midSideLabel.setJustificationType(juce::Justification::centred);

    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    outputLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    linkLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    bypassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    autoGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    oversamplingLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    midSideLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    oversamplingBox.addItem("1x", 1);
    oversamplingBox.addItem("2x", 2);
    oversamplingBox.addItem("4x", 3);
    oversamplingBox.setSelectedId(1);

}

GlobalControlComponent::~GlobalControlComponent()
{
    mixKnob.setLookAndFeel(nullptr);
    outputKnob.setLookAndFeel(nullptr);
    linkButton.setLookAndFeel(nullptr);
    bypassButton.setLookAndFeel(nullptr);
    autoGainButton.setLookAndFeel(nullptr);
    oversamplingBox.setLookAndFeel(nullptr);
    midSideButton.setLookAndFeel(nullptr);

}

void GlobalControlComponent::attachParameters(juce::AudioProcessorValueTreeState& params)
{
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "mix", mixKnob);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "output", outputKnob);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, "bypass", bypassButton);
    autoGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, "autoGain", autoGainButton);
    oversamplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, "oversampling", oversamplingBox);
    midSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, "midSide", midSideButton);
    linkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, "link", linkButton);
n
}

void GlobalControlComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto knobWidth = area.getWidth() / 7;

    auto mixArea = area.removeFromLeft(knobWidth);
    mixKnob.setBounds(mixArea.removeFromTop(knobWidth));
    mixLabel.setBounds(mixArea);

    auto outArea = area.removeFromLeft(knobWidth);
    outputKnob.setBounds(outArea.removeFromTop(knobWidth));
    outputLabel.setBounds(outArea);

    auto linkArea = area.removeFromLeft(knobWidth);
    linkButton.setBounds(linkArea.removeFromTop(knobWidth));
    linkLabel.setBounds(linkArea);

    auto bypassArea = area.removeFromLeft(knobWidth);
    bypassButton.setBounds(bypassArea.removeFromTop(knobWidth));
    bypassLabel.setBounds(bypassArea);

    auto autoArea = area.removeFromLeft(knobWidth);
    autoGainButton.setBounds(autoArea.removeFromTop(knobWidth));
    autoGainLabel.setBounds(autoArea);

    auto osArea = area.removeFromLeft(knobWidth);
    oversamplingBox.setBounds(osArea.removeFromTop(knobWidth / 2));
    oversamplingLabel.setBounds(osArea);

    auto msArea = area;
    midSideButton.setBounds(msArea.removeFromTop(knobWidth));
    midSideLabel.setBounds(msArea);

}
