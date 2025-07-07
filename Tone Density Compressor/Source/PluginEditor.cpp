#include "PluginProcessor.h"
#include "PluginEditor.h"

ToneDensityCompressorAudioProcessorEditor::ToneDensityCompressorAudioProcessorEditor (ToneDensityCompressorAudioProcessor& p)
    : foleys::MagicPluginEditor (p.magicState)
{
    setResizeLimits (400, 300, 1200, 800);
}
