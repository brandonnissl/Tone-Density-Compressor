/*
  ==============================================================================

    SpectralFlatnessGraphComponent.h
    Created: 4 Jul 2025 11:14:55am
    Author:  Brandon Nissl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SpectralFlatnessGraphComponent : public juce::Component, private juce::Timer
{
public:
    SpectralFlatnessGraphComponent();

    void pushNewData(const juce::AudioBuffer<float>& buffer);
    void setCompressionLevels(float low, float mid, float high, float air);
    void setFrequencyRanges(float lL, float lH,
                            float mL, float mH,
                            float hL, float hH,
                            float aL, float aH,
                            double sr);
    bool isShowingPost() const { return prePostToggle.getToggleState(); }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    enum { fftOrder = 9, fftSize = 1 << fftOrder };

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, fftSize> fftData {};
    std::vector<float> magnitudes;
    float compLevels[4] { 0.f, 0.f, 0.f, 0.f };
    float bandLow[4] { 20.f, 250.f, 2000.f, 6000.f };
    float bandHigh[4] { 250.f, 2000.f, 6000.f, 20000.f };
    double sampleRate = 44100.0;

    juce::ToggleButton prePostToggle { "Post" };

    void timerCallback() override { repaint(); }
};
