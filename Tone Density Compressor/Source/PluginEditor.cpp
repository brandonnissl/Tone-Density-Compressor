#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ToneDensityCompressorAudioProcessorEditor::ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    // Attach parameters via public method
    bandsControl.attachParameters(audioProcessor.parameters);
    globalControl.attachParameters(audioProcessor.parameters);

    // Add all components to the editor
    addAndMakeVisible(header);
    addAndMakeVisible(spectralGraph);
    addAndMakeVisible(bandsControl);
    addAndMakeVisible(globalControl);
    addAndMakeVisible(ioMeters);

    // Set plugin window size
    setSize (800, 450);
}

ToneDensityCompressorAudioProcessorEditor::~ToneDensityCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void ToneDensityCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);  // Dark background
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
