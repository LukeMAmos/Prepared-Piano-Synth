/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthVoice.h"
#include "SynthSound.h"

#define numMidiNotes 128
//==============================================================================
/**
*/
class PreparedPianoSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PreparedPianoSynthAudioProcessor();
    ~PreparedPianoSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================

    
    NoteParams& getNoteParams(int noteNumber){
        
        return paramsArray[noteNumber]; 
    }
    
    void setVisualiser(juce::AudioVisualiserComponent* vis){
        
        visualiser = vis;
    }
    
    void updateAll(int currentPos){
        
        //Update all paramters to the current note selected
        for(int i = 0 ; i < numMidiNotes  ; i++){

            paramsArray[i].attack = paramsArray[currentPos].attack;
            paramsArray[i].decay = paramsArray[currentPos].decay;
            paramsArray[i].release = paramsArray[currentPos].release;
            paramsArray[i].sustain = paramsArray[currentPos].sustain;
            
            paramsArray[i].filterType.store(paramsArray[currentPos].filterType.load());
            paramsArray[i].filterResonance.store(paramsArray[currentPos].filterResonance.load());
            paramsArray[i].cutoffFrequency.store(paramsArray[currentPos].cutoffFrequency.load());
            
            paramsArray[i].coe.store(paramsArray[currentPos].coe.load());
            paramsArray[i].roomSize.store(paramsArray[currentPos].roomSize.load());
            paramsArray[i].wetLevel.store(paramsArray[currentPos].wetLevel.load());
            
            paramsArray[i].inputDistortion.store(paramsArray[currentPos].inputDistortion.load());
            paramsArray[i].outputDistortion.store(paramsArray[currentPos].outputDistortion.load());
            
            paramsArray[i].oscType.store(paramsArray[currentPos].oscType.load());
        }
        
    }
    juce::MidiKeyboardState keyboardState;
private:
    
    juce::AudioBuffer<float> currentAudioOut; 
    juce::Synthesiser synth;
    
    //Array of 128 different structures for each of the notes 
    std::array<NoteParams, 128> paramsArray;
    
    std::atomic<juce::AudioVisualiserComponent*> visualiser{nullptr};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreparedPianoSynthAudioProcessor)
};
