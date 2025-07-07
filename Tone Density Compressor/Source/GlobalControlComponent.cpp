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

    addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputKnob);
    addAndMakeVisible(linkButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(mixLabel);
    addAndMakeVisible(outputLabel);
    addAndMakeVisible(linkLabel);
    addAndMakeVisible(bypassLabel);

    mixLabel.setJustificationType(juce::Justification::centred);
    outputLabel.setJustificationType(juce::Justification::centred);
    linkLabel.setJustificationType(juce::Justification::centred);
    bypassLabel.setJustificationType(juce::Justification::centred);
}

GlobalControlComponent::~GlobalControlComponent()
{
    mixKnob.setLookAndFeel(nullptr);
    outputKnob.setLookAndFeel(nullptr);
    linkButton.setLookAndFeel(nullptr);
    bypassButton.setLookAndFeel(nullptr);
}

void GlobalControlComponent::attachParameters(juce::AudioProcessorValueTreeState& params)
{
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "mix", mixKnob);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "output", outputKnob);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, "bypass", bypassButton);
}

void GlobalControlComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto knobWidth = area.getWidth() / 4;

    auto mixArea = area.removeFromLeft(knobWidth);
    mixKnob.setBounds(mixArea.removeFromTop(knobWidth));
    mixLabel.setBounds(mixArea);

    auto outArea = area.removeFromLeft(knobWidth);
    outputKnob.setBounds(outArea.removeFromTop(knobWidth));
    outputLabel.setBounds(outArea);

    auto linkArea = area.removeFromLeft(knobWidth);
    linkButton.setBounds(linkArea.removeFromTop(knobWidth));
    linkLabel.setBounds(linkArea);

    auto bypassArea = area;
    bypassButton.setBounds(bypassArea.removeFromTop(knobWidth));
    bypassLabel.setBounds(bypassArea);
}
