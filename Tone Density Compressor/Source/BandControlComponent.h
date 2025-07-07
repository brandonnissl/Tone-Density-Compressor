/*
  ==============================================================================

    BandControlComponent.h
    Created: 4 Jul 2025 11:15:15am
    Author:  Brandon Nissl

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandControl.h"
#include "LookAndFeel_TDC.h"

class BandControlComponent : public juce::Component
{
public:
    BandControlComponent()
    {
        lowBand.setLookAndFeel (&customLookAndFeel);
        midBand.setLookAndFeel (&customLookAndFeel);
        highBand.setLookAndFeel (&customLookAndFeel);
        airBand.setLookAndFeel (&customLookAndFeel);
        
        addAndMakeVisible(lowBand);
        addAndMakeVisible(midBand);
        addAndMakeVisible(highBand);
        addAndMakeVisible(airBand);
    }
    
     ~BandControlComponent() override
    {
        lowBand.setLookAndFeel (nullptr);
        midBand.setLookAndFeel (nullptr);
        highBand.setLookAndFeel (nullptr);
        airBand.setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override {}
    void resized() override {}
    void attachParameters(juce::AudioProcessorValueTreeState& params)
    {
        lowBand.attachParameters(params, "low");
        midBand.attachParameters(params, "mid");
        highBand.attachParameters(params, "high");
        airBand.attachParameters(params, "air");
    }   

private:
    LookAndFeel_TDC customLookAndFeel;
    BandControl lowBand {"Low"};
    BandControl midBand {"Mid"};
    BandControl highBand {"High"};
    BandControl airBand {"Air"};
};
