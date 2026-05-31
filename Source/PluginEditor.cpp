/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PreparedPianoSynthAudioProcessorEditor::PreparedPianoSynthAudioProcessorEditor (PreparedPianoSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) , visualKeyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    audioProcessor.keyboardState.addListener(this);
    setSize (900, 500);
    setLookAndFeel(&customLookAndFeel);
    
    attack.setRange(0.2, 5.0);
    attack.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    attack.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    attack.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).attack = (float)attack.getValue();
    };
    attack.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(attack);
    
    decay.setRange(0.01, 5.0);
    decay.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    decay.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    decay.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).decay = (float)decay.getValue();
    };
    decay.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(decay);
    
    release.setRange(0.2, 5.0);
    release.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    release.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    release.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).release = (float)release.getValue();
    };
    release.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(release);
    
    sustain.setRange(0.01, 1.0);
    sustain.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    sustain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sustain.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).sustain = (float)sustain.getValue();
    };
    sustain.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(sustain);
    
    filterType.addItem("None", 1);
    filterType.addItem("Lowpass", 2);
    filterType.addItem("Highpass", 3);
    filterType.onChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).filterType.store((FilterType)(filterType.getSelectedId() - 1));
    };
    addAndMakeVisible(filterType);
    
    filterResonance.setRange(0.01, 5.0);
    filterResonance.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterResonance.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    filterResonance.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).filterResonance = (float)filterResonance.getValue();
    };
    filterResonance.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(filterResonance);
    
    cutoffFrequency.setRange(20.0, 20000.0f);
    cutoffFrequency.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    cutoffFrequency.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    cutoffFrequency.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).cutoffFrequency = (float)cutoffFrequency.getValue();
    };
    cutoffFrequency.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(cutoffFrequency);
    
    
    coe.setRange(-1.0 , 1.0);
    coe.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    coe.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    coe.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).coe = (float)coe.getValue();
    };
    coe.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(coe);
    
    
    roomSize.setRange(0.0 , 5.0);
    roomSize.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    roomSize.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    roomSize.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).roomSize = (float)roomSize.getValue();
    };
    roomSize.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(roomSize);
    
    wetLevel.setRange(0.0 , 1.0);
    wetLevel.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wetLevel.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    wetLevel.onValueChange = [this](){
        audioProcessor.getNoteParams(currentMidiNote).wetLevel = (float)wetLevel.getValue();
    };
    wetLevel.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(wetLevel);
    
    //Keyboard on screen
    addAndMakeVisible(visualKeyboard);
    
    //Grouping the components together
    adsrGroup.setText("ADSR");
    addAndMakeVisible(adsrGroup);
    
    filterGroup.setText("Filter");
    addAndMakeVisible(filterGroup);
    
    reverbGroup.setText("Reverb");
    addAndMakeVisible(reverbGroup);
    
    //Labels for each of the dials
    
    //ADSR
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);
    
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(decayLabel);
    
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainLabel);
    
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);
    
    //FILTER
    filterTypeLabel.setText("Filter Type", juce::dontSendNotification);
    filterTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterTypeLabel);
    
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);
    
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceLabel);
    
    //REVERB
    
    for(int i = 0 ; i < 3 ; i++){
        
        reverbLabels[i].setText(reverbLabelNames[i], juce::dontSendNotification);
        reverbLabels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(reverbLabels[i]);
        
    }
    
    
}

PreparedPianoSynthAudioProcessorEditor::~PreparedPianoSynthAudioProcessorEditor()
{
    audioProcessor.keyboardState.removeListener(this);
    setLookAndFeel(nullptr); 
}

//==============================================================================
void PreparedPianoSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PreparedPianoSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //Set positions:
    
    auto area = getLocalBounds().reduced(20); // padding
    auto keyboardArea = area.removeFromBottom(100);
    visualKeyboard.setBounds(keyboardArea);

    auto adsrArea   = area.removeFromLeft(350);
    auto filterArea = area.removeFromLeft(200);
    auto reverbArea = area;
    
    //Adsr
    juce::FlexBox adsrBox;
    adsrBox.flexDirection = juce::FlexBox::Direction::row;
    adsrBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    
    adsrBox.items.add(juce::FlexItem(attack).withWidth(80).withHeight(80));
    adsrBox.items.add(juce::FlexItem(decay).withWidth(80).withHeight(80));
    adsrBox.items.add(juce::FlexItem(sustain).withWidth(80).withHeight(80));
    adsrBox.items.add(juce::FlexItem(release).withWidth(80).withHeight(80));

    
    //Filter
    juce::FlexBox filterBox;
    filterBox.flexDirection = juce::FlexBox::Direction::column;
    filterBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    
    filterBox.items.add(juce::FlexItem(filterType).withWidth(80).withHeight(40).withMargin(juce::FlexItem::Margin(10, 0, 10, 0)));
    filterBox.items.add(juce::FlexItem(cutoffFrequency).withWidth(80).withHeight(80).withMargin(juce::FlexItem::Margin(10, 0, 10, 0)));
    filterBox.items.add(juce::FlexItem(filterResonance).withWidth(80).withHeight(80).withMargin(juce::FlexItem::Margin(10, 0, 10, 0)));

    
    //reverb
    juce::FlexBox reverbBox;
    reverbBox.flexDirection = juce::FlexBox::Direction::row;
    reverbBox.flexWrap = juce::FlexBox::Wrap::wrap;
    
    reverbBox.items.add(juce::FlexItem(coe).withWidth(80).withHeight(80));
    reverbBox.items.add(juce::FlexItem(roomSize).withWidth(80).withHeight(80));
    reverbBox.items.add(juce::FlexItem(wetLevel).withWidth(80).withHeight(80));
    
    //Grouping the components
    adsrGroup.setBounds(adsrArea);
    auto adsrInner = adsrArea.reduced(10);
    adsrInner.removeFromTop(20);
    adsrBox.performLayout(adsrInner.reduced(10).toFloat());
    attackLabel.setBounds(attack.getX(), attack.getY() - 20, attack.getWidth(), 20);
    decayLabel.setBounds(decay.getX(), decay.getY() - 20, decay.getWidth(), 20);
    sustainLabel.setBounds(sustain.getX(), sustain.getY() - 20, sustain.getWidth(), 20);
    releaseLabel.setBounds(release.getX(), release.getY() - 20, release.getWidth(), 20);
    
    
    filterGroup.setBounds(filterArea);
    auto filterInner = filterArea.reduced(10);
    filterInner.removeFromTop(20);
    filterBox.performLayout(filterInner.reduced(10).toFloat());
    filterTypeLabel.setBounds(filterType.getX(), filterType.getY() - 20, filterType.getWidth(), 20);
    cutoffLabel.setBounds(cutoffFrequency.getX(), cutoffFrequency.getY() - 20, cutoffFrequency.getWidth(), 20);
    resonanceLabel.setBounds(filterResonance.getX(), filterResonance.getY() - 20, filterResonance.getWidth(), 20);
    
    
    reverbGroup.setBounds(reverbArea);
    auto reverbInner = reverbArea.reduced(10);
    reverbInner.removeFromTop(20);
    reverbBox.performLayout(reverbInner.reduced(10).toFloat());
    juce::Slider* reverbSliders[3] = {&coe, &roomSize, &wetLevel};

    for(int i = 0; i < 3; i++)
        reverbLabels[i].setBounds(reverbSliders[i]->getX(), reverbSliders[i]->getY() - 20, reverbSliders[i]->getWidth(), 20);
    
}

void PreparedPianoSynthAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity){
    
    juce::MessageManager::callAsync([this, midiNoteNumber](){
        currentMidiNote = midiNoteNumber;
        updateDials(midiNoteNumber);
    });
}

void PreparedPianoSynthAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity){
    
}

void PreparedPianoSynthAudioProcessorEditor::updateDials(int midiNoteNumber){
    
    auto& params = audioProcessor.getNoteParams(midiNoteNumber);
    
    attack.setValue(params.attack , juce::dontSendNotification); //Updating the displayed value on the dial when the current note on changes.
    decay.setValue(params.decay , juce::dontSendNotification) ;
    release.setValue(params.release , juce::dontSendNotification) ;
    sustain.setValue(params.sustain, juce::dontSendNotification) ;
    
    filterType.setSelectedId((int)params.filterType.load()+1 , juce::dontSendNotification);
    filterResonance.setValue(params.filterResonance.load() , juce::dontSendNotification);
    cutoffFrequency.setValue(params.cutoffFrequency.load(), juce::dontSendNotification);
    
    coe.setValue(params.coe.load(), juce::dontSendNotification);
    roomSize.setValue(params.roomSize.load(), juce::dontSendNotification);
    wetLevel.setValue(params.wetLevel.load(), juce::dontSendNotification);
}
