/*
  ==============================================================================

    LookAndFeel_TDC.cpp
    Created: 4 Jul 2025 11:24:53am
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "LookAndFeel_TDC.h"

void LookAndFeel_TDC::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    const float radius = juce::jmin (width, height) / 2.0f - 5.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Background circle
    g.setColour (juce::Colours::darkslategrey);
    g.fillEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Glow ring (outer ring)
    g.setColour (juce::Colours::cyan.withAlpha (0.2f));
    g.drawEllipse (centreX - radius - 2.0f, centreY - radius - 2.0f,
                   (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f, 2.0f);

    // Knob indicator arc
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius - 4.0f, radius - 4.0f,
                            0.0f, rotaryStartAngle, angle, true);

    g.setColour (juce::Colours::cyan);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f));

    // Numeric value (centered inside knob)
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    auto valueString = juce::String (slider.getValue(), 1);
    g.drawFittedText (valueString, x, y, width, height, juce::Justification::centred, 1);
}
