#ifndef FOLEYS_GUI_MAGIC_H_INCLUDED
#define FOLEYS_GUI_MAGIC_H_INCLUDED

#include <JuceHeader.h>

namespace foleys {

class MagicProcessorState;

class MagicPluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit MagicPluginEditor (MagicProcessorState& state);
    ~MagicPluginEditor() override = default;

    void paint (juce::Graphics& g) override {}
    void resized() override {}

private:
    MagicProcessorState& magicState;
};

class MagicProcessorState
{
public:
    MagicProcessorState (juce::AudioProcessor& proc, juce::AudioProcessorValueTreeState& vts)
        : processor (proc)
    {
        juce::ignoreUnused (vts);
    }

    juce::AudioProcessor& getProcessor() { return processor; }

    juce::AudioProcessorEditor* createEditor()
    {
        return new MagicPluginEditor (*this);
    }

private:
    juce::AudioProcessor& processor;
};

inline MagicPluginEditor::MagicPluginEditor (MagicProcessorState& state)
    : juce::AudioProcessorEditor (&state.getProcessor()), magicState (state)
{
    setSize (400, 300);
}

} // namespace foleys

#endif // FOLEYS_GUI_MAGIC_H_INCLUDED
