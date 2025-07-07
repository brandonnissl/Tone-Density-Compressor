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
    SpectralFlatnessGraphComponent() {
        startTimerHz (30); // Refresh rate ~30 fps
    }
    
    void pushNewData(const juce::AudioBuffer<float>& buffer)
        {
            // Simple: compute RMS per channel and store in flatnessValues
            flatnessValues.clear();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                float rms = buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
                flatnessValues.push_back(juce::jlimit(0.0f, 1.0f, rms));
            }
        }
    
    void paint (juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colours::darkslategrey);
        g.fillRoundedRectangle (bounds, 10.0f);

        auto graphArea = bounds.reduced (10.0f);

        g.setColour (juce::Colours::aqua);

        int numBands = static_cast<int> (flatnessValues.size());
        float bandWidth = graphArea.getWidth() / numBands;

        for (int i = 0; i < numBands; ++i)
        {
            float barHeight = graphArea.getHeight() * flatnessValues[i];
            float x = graphArea.getX() + i * bandWidth;
            float y = graphArea.getBottom() - barHeight;

            g.fillRect (x, y, bandWidth * 0.8f, barHeight);
        }
    
    }
    void resized() override {}

private:
    // Add analyzer drawing elements here
    std::vector<float> flatnessValues { 0.3f, 0.5f, 0.4f, 0.6f, 0.35f, 0.55f, 0.45f, 0.5f };

    void timerCallback() override
    {
        repaint();
    }
};
