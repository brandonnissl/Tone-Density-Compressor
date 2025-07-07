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
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        g.setColour (button.getToggleState() ? juce::Colours::aqua : juce::Colours::darkgrey);
        g.fillEllipse (bounds);

        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            g.setColour (juce::Colours::white.withAlpha (0.2f));
            g.drawEllipse (bounds, 2.0f);
        }
    }
};
