/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/**
*/
class PreparedPianoSynthAudioProcessorEditor  : public juce::AudioProcessorEditor ,  public juce::MidiKeyboardState::Listener
{
public:
    PreparedPianoSynthAudioProcessorEditor (PreparedPianoSynthAudioProcessor&);
    ~PreparedPianoSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    
    void updateDials(int midiNoteNumber); 

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PreparedPianoSynthAudioProcessor& audioProcessor;

    juce::MidiKeyboardComponent visualKeyboard;

    
    int currentMidiNote = 60;
    
    //Dials to control the values
    juce::Slider attack;
    juce::Slider decay ;
    juce::Slider release ;
    juce::Slider sustain ;
    
    juce::ComboBox filterType; //0 = none , 1 = lowpass, 2 = highpass
    juce::Slider filterResonance;
    juce::Slider cutoffFrequency;
    
    juce::Slider coe;
    juce::Slider roomSize;
    juce::Slider wetLevel;
    
    //Grouping dials for UI
    juce::GroupComponent adsrGroup;
    juce::GroupComponent filterGroup;
    juce::GroupComponent reverbGroup;
    
    //Labels for the dials
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label filterTypeLabel , resonanceLabel, cutoffLabel;
    juce::Label reverbLabels[4];
    juce::String reverbLabelNames[4] = {"COE", "Room Size" , "Wet level"};
    
    //Custom look
    CustomLookAndFeel customLookAndFeel; 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreparedPianoSynthAudioProcessorEditor)
};
