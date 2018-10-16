using namespace std;
#include "AudioPluginInterface.h"
#include "AudioPluginUtil.h"
#include "Stk.h"
#include "Echo.h"
#include <tgmath.h>
#include <iostream>
#include <fstream>

#include <algorithm>

using namespace stk;




namespace Plugin_SimpleDelay {
    
    ofstream myFile;
    
    class BasicBuffer
    {
    private:
        int bufferSize; // Maximum delay lenght
        float* reverbBufferL ={0};
        float* reverbBufferR ={0};
        int sampleRate ;
        int reverbBufferPosL;
        int reverbBufferPosR;
        
    public:
        BasicBuffer()
        {
            //myFile<< "noParam consturctor called" << endl;
            sampleRate = 44100;
            bufferSize = sampleRate*10;
            
        }
        
        BasicBuffer(int sampleRateX, float bufferLengthInSeconds)
        {
            //myFile << "param consturcotor called " << sampleRateX << " " << bufferLengthInSeconds << endl;
            sampleRate = sampleRateX;
            bufferSize = bufferLengthInSeconds*sampleRate;
            reverbBufferPosL = 0;
            reverbBufferPosR = 0;
            
            free(reverbBufferL);
            free(reverbBufferR);
            //reverbBuffer = dummyArray;
            
            reverbBufferL = (float*) malloc(bufferSize*sizeof(float));
            reverbBufferR =(float*) malloc(bufferSize*sizeof(float));
            
            for(int i = 0 ; i< bufferSize; i++)
            {
                reverbBufferL[i] = 0;
                reverbBufferR[i] = 0;
               // myFile<< reverbBuffer[i] + i << " ";
            }
            
            //myFile << "real reverbBufferSize: " <<sizeof(reverbBufferL) << " saved: " << bufferSize * sizeof(float)<< endl ;
            
           
            
        }
        
        void SetNextValue(float value, int channel)
        {
            if(channel == 0)
            {
                reverbBufferL[reverbBufferPosL] = value;
                
            }
            
            if(channel == 1)
            {
                reverbBufferR[reverbBufferPosR] = value;
                
            }
            // << "set next value " << reverbBufferPos << " " << bufferSize << " : " << reverbBuffer[reverbBufferPos] << endl;
            
        }
        
        float GetValue(int delayInSamples, int channel)
        {
            int id = 0;
            //myFile << "get value called with param: " << delayInSamples << endl;
            if(channel == 0)
                id = reverbBufferPosL - delayInSamples;
            if(channel == 1)
                id = reverbBufferPosR - delayInSamples;
            
            if(id < 0)
                id+= bufferSize;
            //myFile << "get value at: " << id  << " / " << bufferSize  <<endl;
            
            if(channel == 0)
                 return(reverbBufferL[id]);
            if(channel == 1)
                return(reverbBufferR[id]);
            else
                return 0;
                
           
            
        }
        
        void SetParameters(int sampleRateX, float bufferLengthInSeconds)
        {
            sampleRate = sampleRateX;
            bufferSize = bufferLengthInSeconds*sampleRate;
            reverbBufferL[bufferSize] = {0};
            reverbBufferPosL = 0;
            reverbBufferR[bufferSize] = {0};
            reverbBufferPosR = 0;
        }
        
        void Iterate(int channel)
        {
            //myFile << "iterate: " << reverbBufferPos << " / "<< bufferSize<<endl;
            
            if(channel == 0){
            reverbBufferPosL ++;
            if(reverbBufferPosL >= bufferSize)
                reverbBufferPosL = 0;
                
            }
            if(channel == 1){
                reverbBufferPosR ++;
                if(reverbBufferPosR >= bufferSize)
                    reverbBufferPosR = 0;
                
            }
            
        }
    
        
    };
    
    
    /*
    const int reverbBufferSize = 44100*10; // Maximum delay lenght
    float reverbBuffer[reverbBufferSize] = {0};
    int reverbBufferPos = 0;
    float rem = 0;
    */
    
    BasicBuffer* myBuff;
    BasicBuffer* myBuff2;
    BiquadFilter FilterHP[2];
    
    enum Param
    {   P_PAN,
        P_TIME,
        P_GAIN,
        P_HPFREQ,
        P_NUM
        
    
        
    };
    
    struct EffectData
    {
        float p[P_NUM]; // Parameters
    };
    
    
    
    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition [numparams];
        RegisterParameter(definition, "Gain Multiplier", "-",
                          0, 1.0f, 1.0f,
                          1.0f, 1.0f,
                          P_GAIN);
        RegisterParameter(definition,"Delay time" , "ms", 0, 2000, 300, 1.0, 1.0, P_TIME);
        
        RegisterParameter(definition,"Echo pan", " ", -1,1,0,1.0,1.0,P_PAN);
        
        RegisterParameter(definition,"High Pass Freq", "Hz",0,10000,100,1.0,3.0,P_HPFREQ);
        
        return numparams;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        myFile.open("reverbDebugLog.txt");
        //myFile << "CreateCallback called"<< endl ;
        
        EffectData* effectdata = new EffectData;
        
        memset(effectdata, 0, sizeof(EffectData));
        
        myBuff = new BasicBuffer(state->samplerate,5);
        myBuff2 = new BasicBuffer(state->samplerate,5);
        
        
        effectdata->p[P_GAIN] = 1.0f;   // Set default parameter value(s)
        effectdata->p[P_TIME] = 300;
        effectdata->p[P_PAN] = 0;
        state->effectdata = effectdata; // Add our effectdata to the state so we can reach it in other callbacks
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        srand(time(nullptr)); // Seed the random number generator
        
        
        
        
        return UNITY_AUDIODSP_OK;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        delete data;
        delete myBuff;
        
        return UNITY_AUDIODSP_OK;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        data->p[index] = value;
        return UNITY_AUDIODSP_OK;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char *valuestr)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        if (value != NULL)
            *value = data->p[index];
        if (valuestr != NULL)
            valuestr[0] = 0;
        return UNITY_AUDIODSP_OK;
    }
    
    int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
    {
        return UNITY_AUDIODSP_OK;
    }
    float clamp(float x,  float lower,float upper)
    {
        //myFile<< x << " " << upper << " " << lower << endl;
        if(x > upper)
            return upper;
        if(x< lower)
            return lower;
        else
        {
            //myFile << "returning " << x << endl;
            return x;
        }
       
    }
    
    void SetFilterParams(EffectData* data,float samplerate)
    {
        for(int i = 0; i < 2; i++)
        {
            FilterHP[i].SetupHighpass(data->p[P_HPFREQ], samplerate, 1);
        }
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(
      UnityAudioEffectState* state,
      float* inbuffer, float* outbuffer,
      unsigned int length,
      int inchannels, int outchannels)
    {
        
        //myFile<< "processCallback Called" << endl ;
        
        // Grab the EffectData struct we created in CreateCallback()
        EffectData *data = state->GetEffectData<EffectData>();
        
        
        SetFilterParams(data,state->samplerate);
        
        // A gain multiplier to silence the plugin when not in play mode or muted
        float wetTarget = ((state->flags & UnityAudioEffectStateFlags_IsPlaying) && !(state->flags & (UnityAudioEffectStateFlags_IsMuted | UnityAudioEffectStateFlags_IsPaused))) ? 1.0f : 0.0f;
        
        
        
        // For each sample going to the output buffer
        for(unsigned int n = 0; n < length; n++)
        {
            
            
            // For each channel of the buffer
            for(int i = 0; i < outchannels; i++)
            {
                
                
                float channelPanAmp = 1;
                if(i == 0)
                {
                    channelPanAmp = clamp(1.0f-data->p[P_PAN],0.0f,1.0f);
                }
                if(i == 1)
                {
                    channelPanAmp = clamp(1.0f+data->p[P_PAN],0.0f,1.0f);
                }
                
                //myFile<<channelPanAmp << endl;
                
                //reverbBuffer[reverbBufferPos] = inbuffer[n*inchannels + i] * channelPanAmp;
                
                myBuff->SetNextValue( inbuffer[n*inchannels + i],i );
                
                /*
                //Feedback
                if(n*inchannels + i>0)
                {
                     reverbBuffer[reverbBufferPos] = inbuffer[n*inchannels + i] + outbuffer[n*inchannels + i - 1] * data->p[P_GAIN] ;
                }
                else
                {
                     reverbBuffer[reverbBufferPos] = inbuffer[n*inchannels + i] + rem * data->p[P_GAIN];
                }
                */
                
                // NORMALNI DELAY
                /*
                reverbBufferPos ++;
                if(reverbBufferPos == reverbBufferSize-1  || reverbBufferPos > data->p[P_TIME] * 44.1f  )
                {
                    reverbBufferPos = 0;
                }
                */
                //outbuffer[n*outchannels + i] = inbuffer[n*inchannels + i] + reverbBuffer[reverbBufferPos]* data->p[P_GAIN];
                
                int delayInSamples =(data->p[P_TIME]) * 44.1;
                
                float ret = myBuff->GetValue(delayInSamples,i)* data->p[P_GAIN];
                ret = FilterHP[i].Process(ret);
                float ret2 = myBuff->GetValue(delayInSamples*2,i)*data->p[P_GAIN];
                
                
                float retFB = myBuff2->GetValue(delayInSamples,i)*data->p[P_GAIN];
                
                float y = inbuffer[n*inchannels + i] + ret /*+ ret2*/ ;
                
                outbuffer[n*outchannels + i] = y;
                
                
                
                myBuff2->SetNextValue(outbuffer[n*outchannels + i],i);
                
                myBuff->Iterate(i);
                myBuff2->Iterate(i);
                //rem = outbuffer[n*outchannels+i];
               
                // Write the sample to the buffer
                //outbuffer[n * outchannels + i] = inbuffer[n*inchannels + i] * 20.0* log(data->p[P_GAIN]/94);
            }
        }
        
        return UNITY_AUDIODSP_OK;
    }
    
    
    
    
}
