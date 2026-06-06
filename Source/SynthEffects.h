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

//Take the input in multiple it by a gain value softly
//To implement a hard clipped distortion you need to first need to apply a gain , then hard clip the audio , after hard clipping the wave needs to be shaped using a shaping function then an output gain applied to reduce the level of the signal back down to its original input level
class SoftDistortion{
    
public:
    
    void prepare(double sampleRateIn);
    
    void setParameters(float inputGainIn , float outputGainIn){
        
        inputGain = inputGainIn;
        outputGain = outputGainIn;
    }
    
    float process(float input){
        
        //Take the input multiply by the gain , if its over 1 set it to 1 , if its under minus 1 set it to -1
        
        input *= inputGain;
        
        if(input > 1 )
            input = 1;
        else if(input < -1)
            input = -1;
        
        //Shape the rest of the signal
        //apply the output gain and return the value
        
        input = std::sin(0.5 * M_PI * input);
        
        return (input * outputGain);
    }

private:
    
    float inputGain;
    float outputGain;
    
};



enum FilterTypeBiquad{
    
    lowpass,
    highpass,
    bandpass,
    bandreject
};

class BiquadFilter{
    
public:
    
    void prepare(double sampleRateIn){
        
        sampleRate = sampleRateIn;
        
        x1 = x2 = 0.0f;
        y1 = y2 = 0.0f;
        
    }
    
    void setParameters(float cutoffFreqIn , float resonanceIn , FilterTypeBiquad filterTypeIn){
        
        filterType = filterTypeIn;
        resonance = std::max(resonanceIn , (float)std::sqrt(0.5));
        cutoffFrequency = juce::jlimit(0.0f , (sampleRate * 0.5f), cutoffFreqIn);
        
        theta = (( 2.0f * juce::MathConstants<float>::pi * cutoffFrequency ) / sampleRate);
        
        if(filterType == lowpass || filterType == highpass){
            b2 = (2.0f * resonance - sin(theta)) / (2.0f * resonance + sin(theta));
            b1 = -(1 + b2) * cos(theta);
        }else if (filterType == bandpass || filterType == bandreject){
            
            b2 = tan((juce::MathConstants<float>::pi)/(4.0f)-(theta / (2.0f * resonance)));
            b1 = -(1 + b2) * cos(theta);
        }
        switch (filterType) {
            case lowpass:
                a0 = 0.25 * (1 + b1 + b2);
                a1 = 2 * a0;
                a2 = a0;
                break;
                
            case highpass:
                a0 = 0.25 * (1 - b1 + b2);
                a1 = -2 * a0;
                a2 = a0;
                break;
                
            case bandpass:
                a0 = 0.5 * (1-b2);
                a1 = 0;
                a2 = -a0;
                break;
                
            case bandreject:
                a0 = 0.5 * (1+b2);
                a1 = 0;
                a2 = a0;
                break;
            
            default:
                break;
        }
        
    }
    
    
    float process(float input){
        
        //y(n) = a0 x(n) + a1 (n-1) + a2 (x-2) - b1 y(n-1) - b2 y(n-2)
        //Filter coeffients are calculated from cutoff , resonance and sampleRate, a biquad filter can act as both a lowpass and a highpass filter dependant on the input values
        
        float output = a0 * input +a1 * x1 +a2 * x2 -b1 * y1 - b2 * y2;
        
        x2 = x1;
        x1 = input;
        
        y2 = y1;
        y1 = output;
        
        return output;
    }
    
private:
    
    //For a biquad filter need to know maximum 2 samples prior
    float x1 = 0.0f;
    float x2 = 0.0f;

    float y1 = 0.0f;
    float y2 = 0.0f;
    
    FilterTypeBiquad filterType; //Select between lowpass and highPassFilter, flipped signs on highpassfilter
    float sampleRate;
    float cutoffFrequency; //Fc
    float resonance; //(Q)
    
    //Filter Coefficients and variables for calculating
    
    float a0 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
    
    float b1 = 0.0f;
    float b2 = 0.0f;
    
    float theta = 0.0f;
    
};
