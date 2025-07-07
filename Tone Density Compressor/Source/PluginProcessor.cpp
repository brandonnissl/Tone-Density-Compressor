#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioBuffer<float> analysisBuffer;
//==============================================================================
ToneDensityCompressorAudioProcessor::ToneDensityCompressorAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", createParameterLayout())
{
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
    
    analysisBuffer.setSize(2, 512);  // Stereo, 512 samples (adjustable)

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
    const int numSamples = buffer.getNumSamples();
    const int numSamplesToCopy = juce::jmin(buffer.getNumSamples(), 512);

    analysisBuffer.setSize(buffer.getNumChannels(), numSamplesToCopy);
    dryBuffer.makeCopyOf(buffer);

    auto lowBypass = parameters.getRawParameterValue("lowBypass")->load() > 0.5f;
    auto midBypass = parameters.getRawParameterValue("midBypass")->load() > 0.5f;
    auto highBypass = parameters.getRawParameterValue("highBypass")->load() > 0.5f;
    auto airBypass = parameters.getRawParameterValue("airBypass")->load() > 0.5f;

    auto mix = parameters.getRawParameterValue("mix")->load() / 100.0f;
    auto outputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
    bool bypass = parameters.getRawParameterValue("bypass")->load() > 0.5f;

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        if (! bypass)
            buffer.applyGain(ch, 0, numSamples, outputGain);

        analysisBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamplesToCopy);
    }

    for (int ch = totalNumInputChannels; ch < totalNumOutputChannels; ++ch)
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

    auto addBandParams = [&params](const juce::String& prefix) {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Sensitivity", prefix + " Sensitivity", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "Compression", prefix + " Compression", -24.0f, 24.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "Bypass", prefix + " Bypass", false));
    };

    addBandParams("low");
    addBandParams("mid");
    addBandParams("high");
    addBandParams("air");

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToneDensityCompressorAudioProcessor();
}
