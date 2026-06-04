#include "SynthVoice.h"

SynthVoice::SynthVoice(std::array<NoteParams, 128>* paramsArray) : paramsArray(paramsArray) {}

void SynthVoice::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    sineOsc.prepare(spec);
    sineOsc.initialise([](float x){return x < 0.0f ? -1.0f : 1.0f;});
    
    adsr.setSampleRate(spec.sampleRate);
    
    for(int i = 0 ; i < 2 ; i++){
        reverb[i].prepare(sampleRate , 2000);
    }
    filter.prepare(sampleRate);
    
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
    updateValues(midiNoteNumber);
    recentMidiNote = midiNoteNumber;

    sineOsc.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    
    adsr.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{

        adsr.noteOff();
        ringReverb = true;

}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    juce::ScopedNoDenormals noDenormals;

    privateBuffer.clear();

    juce::dsp::AudioBlock<float> block(privateBuffer);
    auto subBlock = block.getSubBlock(0, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(subBlock);

    sineOsc.process(context);
    adsr.applyEnvelopeToBuffer(privateBuffer, 0, numSamples);

    
    for(int ch = 0 ; ch < outputBuffer.getNumChannels() ; ch++){
        
        for(int s = 0 ; s < numSamples ; s++){
            
            privateBuffer.setSample(ch, s, (reverb[ch].process(filter.process(privateBuffer.getSample(ch, s)))));
            
        }
    }
    
    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom(ch, startSample, privateBuffer, ch, 0, numSamples);
}

void SynthVoice::pitchWheelMoved(int newValue) {}
void SynthVoice::controllerMoved(int controllerNumber, int newValue) {}

void SynthVoice::updateValues(int midiNoteNumber)
{
    juce::ADSR::Parameters adsrParams;
    
    auto& data = paramsArray->at(midiNoteNumber);
    adsrParams.attack  = data.attack;
    adsrParams.decay   = data.decay;
    adsrParams.sustain = data.sustain;
    adsrParams.release = data.release;
    adsr.setParameters(adsrParams);
    
    float coe = data.coe.load();
    float roomSize = data.roomSize.load();
    float wetAm = data.wetLevel.load();

    for(int i = 0 ; i < 2 ; i++){
        reverb[i].setParameters(coe,2500.0f, roomSize, wetAm);
    }
    
    float cutoff = data.cutoffFrequency.load();
    float resonance = data.filterResonance.load();
    auto type = data.filterType.load();
    
    filter.setParameters(cutoff, resonance, type);
}
