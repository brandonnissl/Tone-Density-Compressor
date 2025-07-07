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

    addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputKnob);
    addAndMakeVisible(linkButton);
    addAndMakeVisible(mixLabel);
    addAndMakeVisible(outputLabel);
    addAndMakeVisible(linkLabel);

    mixLabel.setJustificationType(juce::Justification::centred);
    outputLabel.setJustificationType(juce::Justification::centred);
    linkLabel.setJustificationType(juce::Justification::centred);
}

GlobalControlComponent::~GlobalControlComponent()
{
    mixKnob.setLookAndFeel(nullptr);
    outputKnob.setLookAndFeel(nullptr);
    linkButton.setLookAndFeel(nullptr);
}

void GlobalControlComponent::attachParameters(juce::AudioProcessorValueTreeState& params)
{
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "mix", mixKnob);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, "output", outputKnob);
}

void GlobalControlComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto knobWidth = area.getWidth() / 3;

    auto mixArea = area.removeFromLeft(knobWidth);
    mixKnob.setBounds(mixArea.removeFromTop(knobWidth));
    mixLabel.setBounds(mixArea);

    auto outArea = area.removeFromLeft(knobWidth);
    outputKnob.setBounds(outArea.removeFromTop(knobWidth));
    outputLabel.setBounds(outArea);

    auto linkArea = area;
    linkButton.setBounds(linkArea.removeFromTop(knobWidth));
    linkLabel.setBounds(linkArea);
}
