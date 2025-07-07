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

    juce::Colour baseColour = juce::Colours::darkslategrey;
    juce::Colour highlight = juce::Colours::aqua;

    juce::Path knob;
    knob.addEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (baseColour);
    g.fillPath (knob);

    g.setColour (baseColour.brighter(0.2f));
    g.strokePath (knob, juce::PathStrokeType (1.0f));

    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius - 4.0f, radius - 4.0f,
                            0.0f, rotaryStartAngle, angle, true);
    g.setColour (highlight);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f));

    juce::Point<float> pointer (centreX + std::cos(angle) * (radius - 6.0f),
                               centreY + std::sin(angle) * (radius - 6.0f));
    g.setColour (highlight);
    g.fillEllipse (pointer.x - 3.0f, pointer.y - 3.0f, 6.0f, 6.0f);
}
