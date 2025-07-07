/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class ToneDensityCompressorAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    ToneDensityCompressorAudioProcessor();
    ~ToneDensityCompressorAudioProcessor() override;

    //==============================================================================

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    juce::AudioProcessorValueTreeState parameters;


    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    const juce::AudioBuffer<float>& getAnalysisBuffer() const { return analysisBuffer; }

private:
    // DSP Filters for Bands
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowBandFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> midBandFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highBandFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> airBandFilter;

    juce::AudioBuffer<float> dryBuffer;  // For dry/wet mixing
    juce::AudioBuffer<float> analysisBuffer; // Captures data for the spectral graph

    juce::dsp::Oversampling<float> oversampler { 2, 4, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneDensityCompressorAudioProcessor)
};
