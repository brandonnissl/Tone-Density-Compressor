/*
  ==============================================================================

    HeaderComponent.cpp
    Created: 4 Jul 2025 11:20:06am
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "HeaderComponent.h"

HeaderComponent::HeaderComponent()
{
    logoLabel.setText("Tone Density Compressor", juce::dontSendNotification);
    logoLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    logoLabel.setJustificationType(juce::Justification::centredLeft);
    logoLabel.setColour(juce::Label::textColourId, juce::Colours::white);


    addAndMakeVisible(logoLabel);
    addAndMakeVisible(presetBox);
    addAndMakeVisible(settingsButton);
}

void HeaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient grad(juce::Colours::black, bounds.getTopLeft(),
                              juce::Colours::darkslategrey, bounds.getBottomLeft(), false);
    g.setGradientFill(grad);
    g.fillRect(bounds);
}

void HeaderComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    logoLabel.setBounds(area.removeFromLeft(200));
    settingsButton.setBounds(area.removeFromRight(30));
    presetBox.setBounds(area);
}
