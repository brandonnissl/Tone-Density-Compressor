/*
  ==============================================================================

    DotKnob.cpp
    Created: 7 Jul 2025 7:55:18pm
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "DotKnob.h"

DotKnob::DotKnob(const juce::String& labelText, int numDots)
    : label(labelText), numDots(numDots)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.addListener(this);
    addAndMakeVisible(slider);

    label.setJustificationType(juce::Justification::bottom);
    auto area = getLocalBounds().reduced(10);
    auto labelHeight = 20;
    label.setBounds(area.removeFromTop(labelHeight));
    addAndMakeVisible(label);

    startTimerHz(30);  // Smooth animation timer
}

void DotKnob::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto labelHeight = 20;
    label.setBounds(area.removeFromTop(labelHeight));  // Label ABOVE the knob
    slider.setBounds(area);
}

void DotKnob::paint(juce::Graphics& g)
{
    auto bounds = slider.getBounds().toFloat().reduced(20.0f);
    auto center = bounds.getCentre();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 10.0f;

    float angleStart = juce::MathConstants<float>::pi * 1.25f;
    float angleEnd = juce::MathConstants<float>::pi * 2.75f;

    for (int i = 0; i < numDots; ++i)
    {
        float angle = juce::jmap<float>(i, 0, numDots - 1, angleStart, angleEnd);
        float x = center.x + radius * std::cos(angle);
        float y = center.y + radius * std::sin(angle);

        float angleDistance = std::abs(angle - currentAngle);
        float maxDistance = (angleEnd - angleStart) / 2.0f;
        float glowStrength = juce::jlimit(0.0f, 1.0f, 1.0f - (angleDistance / maxDistance));  // Falloff

        // Glow effect (multi-dot soft glow)
        if (glowStrength > 0.05f)
        {
            g.setColour(juce::Colours::limegreen.withAlpha(0.2f * glowStrength));
            float glowSize = 20.0f * glowStrength;
            g.fillEllipse(x - glowSize * 0.5f, y - glowSize * 0.5f, glowSize, glowSize);
        }

        // Dot itself
        g.setColour(juce::Colours::dimgrey);
        g.fillEllipse(x - 3.0f, y - 3.0f, 6.0f, 6.0f);
    }
}

void DotKnob::sliderValueChanged(juce::Slider* s)
{
    if (s == &slider)
    {
        float angleStart = juce::MathConstants<float>::pi * 1.25f;
        float angleEnd = juce::MathConstants<float>::pi * 2.75f;

        targetAngle = juce::jmap<float>(slider.getValue(), slider.getMinimum(), slider.getMaximum(), angleStart, angleEnd);
    }
}

void DotKnob::timerCallback()
{
    // Smoothly animate towards target angle
    currentAngle += (targetAngle - currentAngle) * smoothingFactor;
    repaint();
}
