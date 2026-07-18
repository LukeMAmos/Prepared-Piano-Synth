#include "SynthVoice.h"


SynthVoice::SynthVoice(std::array<NoteParams, 128>* paramsArray) : paramsArray(paramsArray) {}

void SynthVoice::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    OSC.prepare(spec);
    OSC.initialise([](float x){return x < 0.0f ? -1.0f : 1.0f;}); //Initalised as a square wave , implementing OSC switching for individual voicing
    
    adsr.setSampleRate(spec.sampleRate);
    
    for(int i = 0 ; i < 2 ; i++){
        reverb[i].prepare(sampleRate , 2000);
        
        filter[i].prepare(sampleRate);
        delay[i].prepare(sampleRate, 8000); //8 Seconds Max delay
    }
    privateBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    privateBuffer.clear();
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return true;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                            juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    //Call to update the parameters 
    updateValues(midiNoteNumber);
    recentMidiNote = midiNoteNumber;

    OSC.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    
    adsr.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{

        adsr.noteOff();
        ringReverb = true; //Allow the reverb to ring out before releasing the note to be used elsewhere

}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    juce::ScopedNoDenormals noDenormals;

    privateBuffer.clear();

    juce::dsp::AudioBlock<float> block(privateBuffer);
    auto subBlock = block.getSubBlock(0, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(subBlock);

    OSC.process(context);
    adsr.applyEnvelopeToBuffer(privateBuffer, 0, numSamples);

    //Applying the effects , to the note 
    for(int ch = 0 ; ch < outputBuffer.getNumChannels() ; ch++){
        
        for(int s = 0 ; s < numSamples ; s++){
            float sample = (reverb[ch].process(filter[ch].process(distortion.process(delay[ch].process(privateBuffer.getSample(ch, s))))));
            privateBuffer.setSample(ch, s, sample);
            
        }
    }
    
    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom(ch, startSample, privateBuffer, ch, 0, numSamples);
}

void SynthVoice::pitchWheelMoved(int newValue) {}
void SynthVoice::controllerMoved(int controllerNumber, int newValue) {}

//Updating the values for the synth voice
void SynthVoice::updateValues(int midiNoteNumber)
{
    juce::ADSR::Parameters adsrParams;
    
    auto& data = paramsArray->at(midiNoteNumber); //getting the data at the specific midi note , then using that data to control the effects/ parameters of that voice
    adsrParams.attack  = data.attack;
    adsrParams.decay   = data.decay;
    adsrParams.sustain = data.sustain;
    adsrParams.release = data.release;
    adsr.setParameters(adsrParams);
    
    float coe = data.coe.load();
    float roomSize = data.roomSize.load();
    float wetAm = data.wetLevel.load();

    float cutoff = data.cutoffFrequency.load();
    float resonance = data.filterResonance.load();
    auto type = data.filterType.load();
    
    float inGainDis = data.inputDistortion.load();
    float outGainDis = data.outputDistortion.load();
    distortion.setParameters(inGainDis, outGainDis);
    
    float delayedSampleLevel = data.delayedSampleLevel.load();
    float delayInMS = data.delayMs.load();

    
    for(int  i = 0 ; i < 2 ; i++){
        
        reverb[i].setParameters(coe,2500.0f, roomSize, wetAm);
        
        filter[i].setParameters(cutoff, resonance, type);
        
        delay[i].setParameters(delayedSampleLevel, delayInMS);
        
    }
    if(previousType != data.oscType){ //Only switch or update the wave if there has been a change if no change then dont update the wave
        updateOscillator(data.oscType);
        previousType = data.oscType;
    }
}

//Function for switching through cases and updating the wave type 
void SynthVoice::updateOscillator(OSCType type){
    
    switch (type) {
        case Sine:
            OSC.initialise([](float x){return std::sin(x * 2 * M_PI);});
            break;
        case Square:
            OSC.initialise([](float x){return x < 0.0f ? -1.0f : 1.0f;});
            break;
        case Triangle:
            OSC.initialise([](float x){return x < 0.5f ? x * 4 - 1 : -x * 4 + 3 ;});
            break;
        default:
            break;
    }
    
    
}
