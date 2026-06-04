/*
  ==============================================================================

    SynthVoice.h
    Created: 25 May 2026 9:49:45pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SynthEffects.h"


struct NoteParams{
    
    float attack = 0.1f;
    float decay = 0.1f;
    float release = 0.1f;
    float sustain = 1.0f;
    
    std::atomic<FilterTypeBiquad> filterType{FilterTypeBiquad::lowpass}; 
    std::atomic<float> filterResonance{0.5f};
    std::atomic<float> cutoffFrequency{2500.0f};
    
    std::atomic<float> coe{0.5f};
    std::atomic<float> roomSize{0.5f};
    std::atomic<float> wetLevel{0.2f};
};


class SynthVoice : public juce::SynthesiserVoice{
    
public:
    
    SynthVoice(std::array<NoteParams, 128>* paramsArray);
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    bool canPlaySound(juce::SynthesiserSound* sound) override;
        
    void startNote(int midiNoteNumber , float velocity , juce::SynthesiserSound* sound , int currentPitchWheelPosition) override;
    
    void stopNote(float velocity , bool allowTailOff) override;
    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer , int startSample , int numSamples) override;
    
    void pitchWheelMoved(int newValue) override;
    
    void controllerMoved(int controllerNumber, int newValue) override;
    
    void updateValues(int midiNoteNumber); 
    
    // Override this so JUCE doesn't cull the voice while reverb is draining
    bool isVoiceActive() const override
    {
        return adsr.isActive() || ringReverb ; 
    }
    
private:
    int recentMidiNote;
    
    juce::dsp::Oscillator<float> sineOsc;
    juce::ADSR adsr;
    
    std::array<NoteParams, 128>* paramsArray; //Settings for every single note on the midi keyboard 
    
    juce::AudioBuffer<float> privateBuffer;
    
    //Track tail off
    bool tailOff = true;
    bool ringReverb = false; 
    int silentBlockCount = 0;
    
    double sampleRate = 44100.0;
    
    //Effects
    
    Reverb reverb[2];
    BiquadFilter filter; 
    
    
};
