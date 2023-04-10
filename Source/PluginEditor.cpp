
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthAudioProcessorEditor::SynthAudioProcessorEditor (SynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (416, 908);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    tensionAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TENSION", tensionSlider);
    toneAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TONE", toneSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);
    sustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SUSTAIN", sustainSlider);

    setSliderParams(tensionSlider, tensionLabel, "Tension");
    setSliderParams(toneSlider, toneLabel, "Tone");
    setSliderParams(gainSlider, gainLabel, "Gain");
    setSliderParams(sustainSlider, sustainLabel, "Sustain");

    Timer::startTimerHz(60);
}

SynthAudioProcessorEditor::~SynthAudioProcessorEditor()
{
}

//==============================================================================
void SynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
    
    Image background = ImageCache::getFromMemory(BinaryData::Pluginbackground_jpg, BinaryData::Pluginbackground_jpgSize);
    background = background.rescaled(416, 908, Graphics::mediumResamplingQuality);
    g.drawImageAt(background, 0, 0);

    numTraste = audioProcessor.getNumTraste();
    numCuerda = audioProcessor.getNumCuerda();
    visualCuerda = audioProcessor.getVisual();
    
    Path p;

    for (int i = 0; i < 16; i++) {
        p.clear();
        
        std::vector<int>::iterator it = std::find(numCuerda.begin(), numCuerda.end(), i);
        int dist = std::distance(numCuerda.begin(), it);

        if (dist < numCuerda.size() && (numTraste.size() == numCuerda.size() && numTraste.size() == visualCuerda.size())) {
            int currentTraste = numTraste[dist];
            int currentCuerda = numCuerda[dist];
            std::vector <float> currentVisual = visualCuerda[dist];

            for (int j = 0; j < currentVisual.size(); j++) {
                if (j == 0) {
                    p.startNewSubPath(4, 0);
                    p.startNewSubPath(-4, 0);
                    p.startNewSubPath(currentVisual[j], 0);
                }
                else
                    p.lineTo(currentVisual[j], j);
            }

            float dist = 1;
            float distPre = 1;

            for (int k = 0; k < currentTraste; k++) {
                distPre = dist;
                dist = dist - dist / 17.817f;
            }
        
            p.scaleToFit(i * getLocalBounds().getWidth() / 16.0f, 0, getLocalBounds().getWidth() / 16.0f, 908 * dist, false);
            g.setColour(Colours::darkgrey);
            g.strokePath(p, PathStrokeType(3.0f * (1.0f / (i / 8.0f + 1.0f))));
            
            p.clear();
            p.startNewSubPath(0, 0);
            p.lineTo(0, 1);
            p.startNewSubPath(-2, 1);
            p.startNewSubPath(2, 1);
            p.scaleToFit(i * getLocalBounds().getWidth() / 16.0f, 908 * dist, getLocalBounds().getWidth() / 16.0f, 908 - 908 * dist, false);
            g.strokePath(p, PathStrokeType(3.0f * (1.0f / (i / 8.0f + 1.0f))));

            g.setColour(Colours::black);
            g.fillEllipse(i * (getLocalBounds().getWidth() / 16.0f) + getLocalBounds().getWidth() / 32.0f - 7.0f, 908 * dist - 5.0f + 908 * (distPre - dist) / 2.0f, 14, 14);
        }
        else {
            p.startNewSubPath(2, 0);
            p.startNewSubPath(-2, 0);
            p.startNewSubPath(0, 0);
            p.lineTo(0, 1);
            p.scaleToFit(i * getLocalBounds().getWidth() / 16.0f, 0, getLocalBounds().getWidth() / 16.0f, 908, false);
            g.setColour(Colours::darkgrey);
            g.strokePath(p, PathStrokeType(3.0f * (1.0f / (i / 9.0f + 1.0f))));
        }
    }
    g.setColour(Colours::darkgrey);
    g.fillRect(0, 0, 416, 110);
}

void SynthAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    const auto sliderWidth = bounds.getWidth() / 6;
    const auto sliderHeight = bounds.getWidth() / 6;
    const auto sliderStartX = bounds.getWidth() / 8 - (sliderWidth / 2);
    const auto sliderStartY = 15 + 92 / 2 - (sliderHeight / 2);

    tensionSlider.setBounds(sliderStartX, sliderStartY, sliderWidth, sliderHeight);
    sustainSlider.setBounds(sliderStartX + bounds.getWidth() / 4, sliderStartY, sliderWidth, sliderHeight);
    toneSlider.setBounds(sliderStartX + 2 * bounds.getWidth() / 4, sliderStartY, sliderWidth, sliderHeight);
    gainSlider.setBounds(sliderStartX + 3 * bounds.getWidth() / 4, sliderStartY, sliderWidth, sliderHeight);
}

void SynthAudioProcessorEditor::setSliderParams(juce::Slider& slider, juce::Label& label, juce::String name) {
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 25);

    label.setJustificationType(juce::Justification::centredBottom);
    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&slider, false);

    addAndMakeVisible(label);
    addAndMakeVisible(slider);
}

void SynthAudioProcessorEditor::timerCallback()
{
    repaint();
}