#include "PluginProcessor.h"
#include "PluginEditor.h"

ToneDensityCompressorAudioProcessorEditor::ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor& p)
    : foleys::MagicPluginEditor (p.magicState)
{
    bandsControl.attachParameters(audioProcessor.parameters);
    globalControl.attachParameters(audioProcessor.parameters);

    addAndMakeVisible(header);
    addAndMakeVisible(spectralGraph);
    addAndMakeVisible(bandsControl);
    addAndMakeVisible(globalControl);
    addAndMakeVisible(ioMeters);

    setSize (800, 450);

}

void ToneDensityCompressorAudioProcessorEditor::timerCallback()
{

}

//==============================================================================
void ToneDensityCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ToneDensityCompressorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto headerHeight = 40;
    auto graphHeight = bounds.getHeight() * 0.4f;
    auto metersHeight = 50;

    auto remainingHeight = bounds.getHeight() - headerHeight - static_cast<int>(graphHeight) - metersHeight;

    auto bandsHeight = static_cast<int>(remainingHeight * 0.55f);
    auto globalHeight = static_cast<int>(remainingHeight * 0.45f);

    header.setBounds(bounds.removeFromTop(headerHeight));
    spectralGraph.setBounds(bounds.removeFromTop(static_cast<int>(graphHeight)));
    bandsControl.setBounds(bounds.removeFromTop(bandsHeight));
    globalControl.setBounds(bounds.removeFromTop(globalHeight));
    ioMeters.setBounds(bounds.removeFromTop(metersHeight));

}
