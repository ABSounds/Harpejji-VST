/*
  ==============================================================================

    SynthSound.h
    Created: 22 Apr 2021 9:05:35am
    Author:  Alberto

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};