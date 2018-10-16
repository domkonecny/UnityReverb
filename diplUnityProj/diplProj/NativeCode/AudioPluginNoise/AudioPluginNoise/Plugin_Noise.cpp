#include "AudioPluginUtil.h"

namespace Plugin_Noise {
    enum Param
    {
        P_GAIN,
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
        RegisterParameter(definition, "Gain Multiplier", "",
                          0.0f, 10.0f, 1.0f,
                          1.0f, 1.0f,
                          P_GAIN);
        return numparams;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        EffectData* effectdata = new EffectData;
        memset(effectdata, 0, sizeof(EffectData));
        effectdata->p[P_GAIN] = 1.0f;   // Set default parameter value(s)
        state->effectdata = effectdata; // Add our effectdata to the state so we can reach it in other callbacks
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        srand(time(nullptr)); // Seed the random number generator
        return UNITY_AUDIODSP_OK;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        delete data;
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
    
    // ProcessCallback() gets called as long as the plugin is loaded
    // This includes when the editor is not in play mode!
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(
                                                                  UnityAudioEffectState* state,
                                                                  float* inbuffer, float* outbuffer,
                                                                  unsigned int length,
                                                                  int inchannels, int outchannels)
    {
        // Grab the EffectData struct we created in CreateCallback()
        EffectData *data = state->GetEffectData<EffectData>();
        
        // A gain multiplier to silence the plugin when not in play mode or muted
        float wetTarget = ((state->flags & UnityAudioEffectStateFlags_IsPlaying) && !(state->flags & (UnityAudioEffectStateFlags_IsMuted | UnityAudioEffectStateFlags_IsPaused))) ? 1.0f : 0.0f;
        
        // For each sample going to the output buffer
        for(unsigned int n = 0; n < length; n++)
        {
            // For each channel of the buffer
            for(int i = 0; i < outchannels; i++)
            {
                // Generate a random float in the range [-1.0, 1.0] (fixed-point arithmetic)
                int rInt = rand() % 20000;
                rInt -= 10000;
                float r = rInt / 10000.0f;
                
                // Write the sample to the buffer
                outbuffer[n * outchannels + i] = r * wetTarget * data->p[P_GAIN];
            }
        }
        
        return UNITY_AUDIODSP_OK;
    }
}
