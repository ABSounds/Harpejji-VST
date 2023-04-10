/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthAudioProcessor::SynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),   apvts (*this, nullptr, "Parameters", createParameters())
                        ,   lpf(dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(48000, 20000.0f))
#endif
{
    synth.addSound(new SynthSound());
    for (int i = 0; i < numVoices; i++) {
        synth.addVoice(new SynthVoice());
    }
}

SynthAudioProcessor::~SynthAudioProcessor()
{
}

//==============================================================================
const juce::String SynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int SynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{   
    juce::dsp::ProcessSpec spec{};
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    synth.setCurrentPlaybackSampleRate(sampleRate);
    synth.setNoteStealingEnabled(false);

    lpf.prepare(spec);
    lpf.reset();

    gain.prepare(spec);
    gain.reset();

    for (int i = 0; i < synth.getNumVoices(); ++i) {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i))) {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
}

void SynthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
        return true;
  #endif
}
#endif

void SynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    dsp::AudioBlock<float> block(buffer);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    visualCuerda.clear();
    numCuerda.clear();
    numTraste.clear();

    for (int i = 0; i < synth.getNumVoices(); ++i) {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            // Aquí se actualizan los parámetros para cada voz
            auto& tension = *apvts.getRawParameterValue("TENSION");
            auto& sustain = *apvts.getRawParameterValue("SUSTAIN");

            voice->updateParams(tension, sustain);

            if (voice->isVoiceActive())
            {
                visualCuerda.push_back(voice->getVisual());
                numCuerda.push_back(voice->getNumCuerda());
                numTraste.push_back(voice->getNumTraste());
            }
        }
    }
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    updateParams();
    lpf.process(dsp::ProcessContextReplacing<float>(block));
    gain.process(dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool SynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SynthAudioProcessor::createEditor()
{
    return new SynthAudioProcessorEditor (*this);
}

//==============================================================================
void SynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void SynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//============================================================================== 
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout SynthAudioProcessor::createParameters(){
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Se crean los parámetros 
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TENSION", "Tension", juce::NormalisableRange<float> { 0.1f, 1.4f, 0.1f }, 1.0f));             // Valores min, max, paso y default de los parámetros
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TONE", "Tone", juce::NormalisableRange<float> { 10.0f, 5000.0f, 10.0f}, 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", juce::NormalisableRange<float> { -60.f, 0.0f, 0.1f}, -12.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f}, 1.0f));

    return { params.begin(),params.end() };
}

void SynthAudioProcessor::updateParams() {
    auto& cutOff = *apvts.getRawParameterValue("TONE");
    *lpf.state = *dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(getSampleRate(), cutOff);

    auto& vol = *apvts.getRawParameterValue("GAIN");
    gain.setGainDecibels(vol);
}

std::vector <int> SynthAudioProcessor::getNumTraste() 
{
    return numTraste;
}

std::vector <int> SynthAudioProcessor::getNumCuerda()
{
    return numCuerda;
}

std::vector <std::vector <float>> SynthAudioProcessor::getVisual()
{
    return visualCuerda;
}