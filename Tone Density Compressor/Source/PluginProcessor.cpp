#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
ToneDensityCompressorAudioProcessor::ToneDensityCompressorAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    oversampler.reset();
}

ToneDensityCompressorAudioProcessor::~ToneDensityCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String ToneDensityCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ToneDensityCompressorAudioProcessor::acceptsMidi() const { return false; }
bool ToneDensityCompressorAudioProcessor::producesMidi() const { return false; }
bool ToneDensityCompressorAudioProcessor::isMidiEffect() const { return false; }
double ToneDensityCompressorAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ToneDensityCompressorAudioProcessor::getNumPrograms() { return 1; }
int ToneDensityCompressorAudioProcessor::getCurrentProgram() { return 0; }
void ToneDensityCompressorAudioProcessor::setCurrentProgram (int) {}
const juce::String ToneDensityCompressorAudioProcessor::getProgramName (int) { return {}; }
void ToneDensityCompressorAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void ToneDensityCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    *lowBandFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 120.0f);
    *midBandFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 500.0f);
    *highBandFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 3000.0f);
    *airBandFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 6000.0f);

    dryBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    preAnalysisBuffer.setSize(getTotalNumInputChannels(), 512);
    analysisBuffer.setSize(2, 512);  // Stereo, 512 samples (adjustable)

    oversampler.initProcessing(samplesPerBlock);
    oversampler.reset();
    currentSampleRate = sampleRate;

}

void ToneDensityCompressorAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ToneDensityCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void ToneDensityCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int channelsToProcess = juce::jmax (buffer.getNumChannels(), totalNumOutputChannels);

    const int numSamples = buffer.getNumSamples();
    const int numSamplesToCopy = juce::jmin(buffer.getNumSamples(), 512);

    analysisBuffer.setSize(buffer.getNumChannels(), numSamplesToCopy);

    if (totalNumInputChannels == 1 && totalNumOutputChannels >= 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);

    dryBuffer.makeCopyOf(buffer);
    const int preSamples = juce::jmin(numSamplesToCopy, preAnalysisBuffer.getNumSamples());
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        preAnalysisBuffer.copyFrom(ch, 0, buffer, ch, 0, preSamples);


    const auto mix = parameters.getRawParameterValue("mix")->load() / 100.0f;
    const float outputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
    const bool bypass = parameters.getRawParameterValue("bypass")->load() > 0.5f;
    const bool autoGain = parameters.getRawParameterValue("autoGain")->load() > 0.5f;
    const int oversampleChoice = (int) parameters.getRawParameterValue("oversampling")->load();
    const bool midSide = parameters.getRawParameterValue("midSide")->load() > 0.5f;


    if (midSide && totalNumInputChannels >= 2)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            auto L = buffer.getSample(0, i);
            auto R = buffer.getSample(1, i);
            buffer.setSample(0, i, 0.5f * (L + R));
            buffer.setSample(1, i, 0.5f * (L - R));
        }
    }

    if (! bypass)
    {
        if (oversampleChoice > 0)
        {
            juce::dsp::AudioBlock<float> block (buffer);
            oversampler.processSamplesUp (block);
            oversampler.processSamplesDown (block);
        }

        for (int ch = 0; ch < channelsToProcess; ++ch)
            buffer.applyGain(ch, 0, numSamples, outputGain);
    }

    for (int ch = 0; ch < channelsToProcess; ++ch)
    {

        auto* dry = dryBuffer.getReadPointer(ch);
        auto* wet = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            wet[i] = wet[i] * mix + dry[i] * (1.0f - mix);

        analysisBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamplesToCopy);
    }

    if (midSide && totalNumInputChannels >= 2)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            auto M = buffer.getSample(0, i);
            auto S = buffer.getSample(1, i);
            buffer.setSample(0, i, M + S);
            buffer.setSample(1, i, M - S);
        }
    }

    if (autoGain && ! bypass)
    {
        float dryRMS = 0.0f;
        float wetRMS = 0.0f;
        for (int ch = 0; ch < channelsToProcess; ++ch)

        {
            dryRMS += dryBuffer.getRMSLevel(ch, 0, numSamples);
            wetRMS += buffer.getRMSLevel(ch, 0, numSamples);
        }

        dryRMS /= static_cast<float> (totalNumInputChannels);
        wetRMS /= static_cast<float> (totalNumInputChannels);


        if (wetRMS > 0.0f)
            buffer.applyGain(dryRMS / wetRMS);
    }

    for (int ch = channelsToProcess; ch < totalNumOutputChannels; ++ch)

        buffer.clear(ch, 0, numSamples);
}


//==============================================================================
bool ToneDensityCompressorAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ToneDensityCompressorAudioProcessor::createEditor()
{
    return new ToneDensityCompressorAudioProcessorEditor (*this);
}

//==============================================================================
void ToneDensityCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ToneDensityCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml.get() != nullptr)
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ToneDensityCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 100.0f, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("autoGain", "Auto Gain", false));
    juce::StringArray osChoices { "1x", "2x", "4x" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("oversampling", "Oversampling", osChoices, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>("midSide", "Mid/Side", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("link", "Link", false));


    auto addBandParams = [&params](const juce::String& prefix,
                                   float defLow,
                                   float defHigh) {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Sensitivity", prefix + " Sensitivity", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Compression", prefix + " Compression", -24.0f, 24.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "Bypass", prefix + " Bypass", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "FreqLow", prefix + " Low Freq", 20.0f, 20000.0f, defLow));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "FreqHigh", prefix + " High Freq", 20.0f, 20000.0f, defHigh));
    };

    addBandParams("low",   20.0f,   250.0f);
    addBandParams("mid",   250.0f,  2000.0f);
    addBandParams("high",  2000.0f, 6000.0f);
    addBandParams("air",   6000.0f, 20000.0f);

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToneDensityCompressorAudioProcessor();
}
