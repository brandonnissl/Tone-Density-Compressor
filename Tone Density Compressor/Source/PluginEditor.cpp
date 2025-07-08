#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ToneDensityCompressorAudioProcessorEditor::ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    startTimerHz(30);

    // Attach parameters via public method
    bandsControl.attachParameters(audioProcessor.parameters);
    globalControl.attachParameters(audioProcessor.parameters);

    // Add all components to the editor
    addAndMakeVisible(header);
    addAndMakeVisible(spectralGraph);
    addAndMakeVisible(bandsControl);
    addAndMakeVisible(globalControl);
    addAndMakeVisible(ioMeters);

    // Ensure the graph sits behind interactive controls
    spectralGraph.toBack();

    // Set plugin window size
    setSize (1200, 600);

}

ToneDensityCompressorAudioProcessorEditor::~ToneDensityCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();

}

//==============================================================================
void ToneDensityCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::grey);  // Dark background
}

void ToneDensityCompressorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto headerHeight = 40;
    auto graphHeight = bounds.getHeight() * 0.4f;  // ~40% of total height for graph
    auto metersHeight = 50;

    auto remainingHeight = bounds.getHeight() - headerHeight - static_cast<int>(graphHeight) - metersHeight;

    auto bandsHeight = static_cast<int>(remainingHeight * 0.55f);   // Bands slightly bigger than global
    auto globalHeight = static_cast<int>(remainingHeight * 0.45f);
    
    header.setBounds(bounds.removeFromTop(headerHeight));
    spectralGraph.setBounds(bounds.removeFromTop(static_cast<int>(graphHeight)));
    bandsControl.setBounds(bounds.removeFromTop(bandsHeight));
    globalControl.setBounds(bounds.removeFromTop(globalHeight));
    ioMeters.setBounds(bounds.removeFromTop(metersHeight));
}

void ToneDensityCompressorAudioProcessorEditor::timerCallback()
{
    if (spectralGraph.isShowingPost())
        spectralGraph.pushNewData(audioProcessor.getAnalysisBuffer());
    else
        spectralGraph.pushNewData(audioProcessor.getPreAnalysisBuffer());

    auto low  = audioProcessor.parameters.getRawParameterValue("lowCompression")->load();
    auto mid  = audioProcessor.parameters.getRawParameterValue("midCompression")->load();
    auto high = audioProcessor.parameters.getRawParameterValue("highCompression")->load();
    auto air  = audioProcessor.parameters.getRawParameterValue("airCompression")->load();

    auto ll = audioProcessor.parameters.getRawParameterValue("lowFreqLow")->load();
    auto lh = audioProcessor.parameters.getRawParameterValue("lowFreqHigh")->load();
    auto ml = audioProcessor.parameters.getRawParameterValue("midFreqLow")->load();
    auto mh = audioProcessor.parameters.getRawParameterValue("midFreqHigh")->load();
    auto hl = audioProcessor.parameters.getRawParameterValue("highFreqLow")->load();
    auto hh = audioProcessor.parameters.getRawParameterValue("highFreqHigh")->load();
    auto al = audioProcessor.parameters.getRawParameterValue("airFreqLow")->load();
    auto ah = audioProcessor.parameters.getRawParameterValue("airFreqHigh")->load();

    spectralGraph.setFrequencyRanges(ll, lh, ml, mh, hl, hh, al, ah, audioProcessor.getCurrentSampleRate());

    spectralGraph.setCompressionLevels(low, mid, high, air);
}
