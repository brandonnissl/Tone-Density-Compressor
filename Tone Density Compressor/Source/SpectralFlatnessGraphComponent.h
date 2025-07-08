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

    void paint (juce::Graphics& g) override;
    void resized() override {}

private:
    enum { fftOrder = 9, fftSize = 1 << fftOrder };

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, fftSize> fftData {};
    std::vector<float> magnitudes;
    float compLevels[4] { 0.f, 0.f, 0.f, 0.f };

    void timerCallback() override { repaint(); }
};
