#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class SynthAudioProcessorEditor  : public juce::AudioProcessorEditor
    ,   public juce::Timer
{
public:
    SynthAudioProcessorEditor (SynthAudioProcessor&);
    ~SynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    void setSliderParams(juce::Slider& slider, juce::Label& label, juce::String name);

    juce::Slider tensionSlider;
    juce::Slider toneSlider;
    juce::Slider gainSlider;
    juce::Slider sustainSlider;

    juce::Label tensionLabel;
    juce::Label toneLabel;
    juce::Label gainLabel;
    juce::Label sustainLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> tensionAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;

    ScopedPointer<Graphics> cuerdaGraphic;

    std::vector <int> numTraste;
    std::vector <int> numCuerda;
    std::vector <std::vector <float>> visualCuerda;
    
    SynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthAudioProcessorEditor)
};
