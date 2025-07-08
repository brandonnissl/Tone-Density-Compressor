#pragma once

#include <JuceHeader.h>

class DotKnob : public juce::Component, private juce::Slider::Listener, private juce::Timer
{
public:
    DotKnob(const juce::String& labelText, int numDots = 30);
    ~DotKnob() override = default;

    void resized() override;
    void paint(juce::Graphics&) override;

    juce::Slider& getSlider() { return slider; }

private:
    juce::Slider slider;
    juce::Label label;
    int numDots;

    float targetAngle = 0.0f;
    float currentAngle = 0.0f;
    float smoothingFactor = 0.15f;  // Controls smoothing speed

    void sliderValueChanged(juce::Slider* s) override;
    void timerCallback() override;
};
