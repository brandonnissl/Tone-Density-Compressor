/*
  ==============================================================================

    SpectralFlatnessGraphComponent.cpp
    Created: 4 Jul 2025 11:14:55am
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "SpectralFlatnessGraphComponent.h"

SpectralFlatnessGraphComponent::SpectralFlatnessGraphComponent()
{
    addAndMakeVisible(prePostToggle);
    prePostToggle.setToggleState(true, juce::dontSendNotification);

    prePostToggle.onClick = [this] { repaint(); };

    setInterceptsMouseClicks(false, true);


    startTimerHz (30);
}

void SpectralFlatnessGraphComponent::pushNewData (const juce::AudioBuffer<float>& buffer)
{
    auto numSamples = juce::jmin<float>(buffer.getNumSamples(), fftSize);

    const float* left  = buffer.getReadPointer (0);
    const float* right = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
        fftData[i] = 0.5f * (left[i] + (right ? right[i] : left[i]));

    for (int i = numSamples; i < fftSize; ++i)
        fftData[i] = 0.0f;

    window.multiplyWithWindowingTable (fftData.data(), fftSize);
    fft.performRealOnlyForwardTransform (fftData.data());

    magnitudes.resize (fftSize / 2);
    for (size_t i = 0; i < magnitudes.size(); ++i)
    {
        auto re = fftData[2 * i];
        auto im = fftData[2 * i + 1];
        float mag = std::sqrt (re * re + im * im);

        float db = juce::Decibels::gainToDecibels (mag / static_cast<float> (fftSize));
        float norm = juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f);
        magnitudes[i] = juce::jlimit (0.0f, 1.0f, norm);
    }
}

void SpectralFlatnessGraphComponent::setCompressionLevels (float low, float mid, float high, float air)
{
    compLevels[0] = low;
    compLevels[1] = mid;
    compLevels[2] = high;
    compLevels[3] = air;
}

void SpectralFlatnessGraphComponent::setFrequencyRanges(float lL, float lH,
                                                        float mL, float mH,
                                                        float hL, float hH,
                                                        float aL, float aH,
                                                        double sr)
{
    bandLow[0] = lL;  bandHigh[0] = lH;
    bandLow[1] = mL;  bandHigh[1] = mH;
    bandLow[2] = hL;  bandHigh[2] = hH;
    bandLow[3] = aL;  bandHigh[3] = aH;
    sampleRate = sr;
}

void SpectralFlatnessGraphComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colours::darkslategrey);
    g.fillRoundedRectangle (bounds, 10.0f);

    bounds.removeFromTop(20.0f);
    auto graphArea = bounds.reduced (10.0f);

    // Draw band ranges background
    for (int i = 0; i < 4; ++i)
    {
        float x1 = graphArea.getX() + (bandLow[i]  / (float)(sampleRate * 0.5)) * graphArea.getWidth();
        float x2 = graphArea.getX() + (bandHigh[i] / (float)(sampleRate * 0.5)) * graphArea.getWidth();
        g.setColour(juce::Colours::whitesmoke.withAlpha(0.08f));
        g.fillRect(juce::Rectangle<float>(x1, graphArea.getY(), x2 - x1, graphArea.getHeight()));
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawLine(x1, graphArea.getY(), x1, graphArea.getBottom());
        g.drawLine(x2, graphArea.getY(), x2, graphArea.getBottom());
    }

    // Draw spectrum
    if (! magnitudes.empty())
    {
        juce::Path p;
        auto width  = graphArea.getWidth();
        auto height = graphArea.getHeight();

        for (size_t i = 0; i < magnitudes.size(); ++i)
        {
            float x = graphArea.getX() + static_cast<float> (i) * width / static_cast<float> (magnitudes.size() - 1);
            float y = graphArea.getBottom() - magnitudes[i] * height;

            if (i == 0)
                p.startNewSubPath (x, y);
            else
                p.lineTo (x, y);
        }

        g.setColour (juce::Colours::aqua);
        g.strokePath (p, juce::PathStrokeType (2.0f));
    }

    // Draw compression level lines
    constexpr float minDb = -24.0f;
    constexpr float maxDb =  24.0f;
    juce::Colour colours[4] { juce::Colours::red, juce::Colours::yellow,
                              juce::Colours::green, juce::Colours::white };

    for (int i = 0; i < 4; ++i)
    {
        float norm = juce::jmap (compLevels[i], minDb, maxDb, 1.0f, 0.0f);
        float y = graphArea.getY() + norm * graphArea.getHeight();
        g.setColour (colours[i]);
        g.drawLine (graphArea.getX(), y, graphArea.getRight(), y, 1.0f);
    }

    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawFittedText("Frequency (Hz)", graphArea.getX(), graphArea.getBottom() + 2,
                     graphArea.getWidth(), 15, juce::Justification::centred, 1);
    g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi,
                                                   graphArea.getX() - 20, graphArea.getCentreY()));
    g.drawFittedText("Amplitude (dB)", graphArea.getX() - 60, graphArea.getY(),
                     50, graphArea.getHeight(), juce::Justification::centred, 1);
}

void SpectralFlatnessGraphComponent::resized()
{
    auto bounds = getLocalBounds();
    prePostToggle.setBounds(bounds.removeFromTop(20).removeFromRight(60));
}
