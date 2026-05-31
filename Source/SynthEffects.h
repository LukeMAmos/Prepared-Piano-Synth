/*
  ==============================================================================

    SynthEffects.h
    Created: 27 May 2026 1:41:37pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class DelayLine{
    
public:
    
    void prepare(double sampleRateIn, float maxDelayMs){
        
        sampleRate = sampleRateIn;
        
        int maxBufferSize = sampleRate * (maxDelayMs / 1000);
        buffer.resize(maxBufferSize, 0);
        writePosition = 0;
        
    }

    void setDelayLine(float ms){
        
        delaySamples = std::min((int)(sampleRate * (ms / 1000.0f)), (int)buffer.size() - 1);
        
    }

    float processSample(float input){
        
        int readPosition = (int)(writePosition - delaySamples + buffer.size()) % buffer.size();
        
        float delayedSample = buffer[readPosition];
        
        buffer[writePosition] = input;
        writePosition = (writePosition + 1 ) % buffer.size();
        
        return delayedSample;
    }
    
    float bufferValue(int samplePos){
        
        return buffer[samplePos];
    }
    
private:
    
    std::vector<float> buffer;
    int writePosition;
    int delaySamples;
    double sampleRate;
    
};

class CombFilter{
    
public:
    
    void prepare(double sampleRateIn ,float maxDelayMs){
        
        delayLine.prepare(sampleRateIn , maxDelayMs);
        sampleRate = sampleRateIn;
        filterState = 0.0f;
    }
    
    void setParameters(float feedbackIn , float cutoffHz , float delayTimeMs){
        
        feedback = juce::jlimit(0.0f , 0.99f , feedbackIn);
        damping = expf(-2.0f * juce::MathConstants<float>::pi * cutoffHz / sampleRate); //Equation to find the damping amount from cutoff frequency
        delayLine.setDelayLine(delayTimeMs);
    }
    
    float processSample(float input){
        
        float delayed = delayLine.processSample(input + filterState * feedback);

        filterState = delayed * (1.0f - damping)
                    + filterState * damping;

        return delayed;
        
    }

private:

    DelayLine delayLine;
    float feedback;
    float sampleRate;
    float damping;
    float filterState;
    
};

//==============================================================================
class AllPassFilter{
    
public:
    
    void prepare(double sampleRateIn , float maxDelayMs){
        int maxSamples = sampleRateIn * (maxDelayMs / 1000);
        
        inputBuffer.resize(maxSamples, 0.0f);
        outputBuffer.resize(maxSamples , 0.0f);
        
        writeIndex = 0;
        delaySamples = 0;
        sampleRate = sampleRateIn;
        
    }
    
    void setParameters(float delayMs , float coefficient){
        
        delaySamples = std::min((int)(sampleRate * (delayMs / 1000)) , (int)(inputBuffer.size() -1 ));
        a = juce::jlimit(-0.99f, 0.99f, coefficient);
        
        std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
        std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
        
    }
    
    float process(float input){
        
        int readIndex = (writeIndex - delaySamples + inputBuffer.size()) % inputBuffer.size();
        
        float xd = inputBuffer[readIndex];
        float yd = outputBuffer[readIndex];
        
        float y = -a * input + xd + a * yd;
        
        inputBuffer[writeIndex] = input;
        outputBuffer[writeIndex] = y;
        
        writeIndex = (writeIndex + 1) % inputBuffer.size();
        
        return y;
    }

private:
    
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    
    int writeIndex = 0;
    int delaySamples = 0;
    float a = 0.0f;
    double sampleRate = 0.0;
};

//Schroeder style reverb built from comb and all pass filters
class Reverb{
    
public:
    
    void prepare(double sampleRate , float maxDelayMs ){
        
        for(int i = 0 ; i < 6 ; i++){
            
            combFilters[i].prepare(sampleRate , maxDelayMs);
        }
        
        for(int i = 0; i < 4 ; i++){
            
            allPassFilters[i].prepare(sampleRate , maxDelayMs);
        }
    }
    
    void setParameters(float coeffiecent , float cutoffHz , float roomSize , float wetAmountIn){
        
        for(int i = 0 ; i < 6 ; i++){
            
            combFilters[i].setParameters(gains[i], cutoffHz, times[i] * roomSize); // roomSize multiples the times values to create a larger preciving reverb
        }
        
        for(int i = 0; i < 4 ; i++){
            
            allPassFilters[i].setParameters(apfTimes[i], coeffiecent);
        }
        
        wetAmount = wetAmountIn;
        
    }
    
    float process(float input){
        
        juce::ScopedNoDenormals noDenormals;
        
        //Shroeder style reverb processes the comb filters in parrellel and then  the all pass filters in series afterwards
        //Loop through the comb filters and store the output value of each in a single variable to be outputted
        float combOut = 0.0f;
        for(int i = 0 ; i < 6; i ++){
            
            combOut += combFilters[i].processSample(input);
            
        }
        
        combOut *= 1.0f / 6.0f; //Normalise the values
        
        //Pass the output of the combFilters into the first apf and then into the next subsequent all pass filter
        
        float a0 = allPassFilters[0].process(combOut);
        float a1 = allPassFilters[1].process(a0);
        float a2 = allPassFilters[2].process(a1);
        float a3 = allPassFilters[3].process(a2);
        
        float output = a3;
        
        return input * (1-wetAmount)+ output * wetAmount;
    }
    
private:
    
    CombFilter combFilters[6];
    AllPassFilter allPassFilters[4];
    
    //All Pass filter times
    float apfTimes[4] = { 5.118, 7.762, 9.957, 12.402 };
    
    //COMB Filter Times
    float times[6] = { 21.884f, 25.885f, 28.934f, 30.739f, 53.229f, 62.829f };
    float gains[6] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    
    float wetAmount = 0.0f;
    
};


class SoftDistortion{
    
public:
    
    void prepare();
    
    void setParameters();
    
    float process(); 

private:

    
};
