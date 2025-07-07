/*
  ==============================================================================

    LookAndFeel_TDC.h
    Created: 4 Jul 2025 11:24:53am
    Author:  Brandon Nissl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LookAndFeel_TDC : public juce::LookAndFeel_V4
{
public:
    LookAndFeel_TDC()
    {
        setColour (juce::Slider::thumbColourId, juce::Colours::aqua);
        setColour (juce::Slider::trackColourId, juce::Colours::darkslategrey);
        setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::aqua);
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::grey);
        setColour (juce::ToggleButton::textColourId, juce::Colours::white);
        setColour (juce::ComboBox::backgroundColourId, juce::Colours::black);
        setColour (juce::ComboBox::textColourId, juce::Colours::white);
        setColour (juce::Label::textColourId, juce::Colours::white);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider& slider) override;
};
