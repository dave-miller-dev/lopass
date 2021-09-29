//
//  LoPassUnit.cpp
//  LoPass
//
//  Created by David Miller on 28/9/21.
//

#include "LoPassUnit.hpp"
#include "AUEffectBase.h"
#include <AudioToolbox/AudioUnitUtilities.h>
#include "LoPassVersion.h"
#include <math.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUDIOCOMPONENT_ENTRY(AUBaseFactory, LoPassUnit)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* The AUDIOCOMPOENT_ENTRY macro is required for the macOS Component Manager to recognize
 and use the audio unit. */

#pragma mark ____ Construction Initialization
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassUnit::LoPassUnit
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The constructor for the new LoPass audio units.
LoPassUnit::LoPassUnit (AudioUnit component) : AUEffectBase(component) {
    
    /* This method, defined in the AUBase superclass, ensures that the required
     audio unit elements are created and initialised. */
    CreateElements();
    
    /* Invokes the use of an STL vector for parameter access.
     See AUBase/AUScopeElement.cpp */
    Globals()->UseIndexedParameters(kNumberOfParameters);
    
    /* All Parameters must be set to their initial values here.
     These calls have the effect of both defining the parameters
     for the first time and assigning their initial values.*/
    
    SetParameter(kParameter_CutoffFrequency, kDefaultValue_LoPass_Frequency);
    SetParameter(kParameter_Resonance, kDefaultValue_LoPass_Resonance);
    
    // Filter Cutoff Frequency max value depends on sample-rate.
    SetParamHasSampleRateDependency(true);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPass Initialise
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//OSStatus LoPassUnit::Initialize() {
//
//    OSStatus result = AUEffectBase::Initialize();
    
//    if (result == noErr) {
//        
//        /* in case the AU was un-initialised and parameters were changed, the view can now
//         be made aware it needs to update the frequency response curve. */
//        PropertyChanged(kAudioUnitCustomProperty_FilterFrequencyResponse, kAudioUnitScope_Global, 0);
//    }
//
//    return result;
//}

#pragma mark ____Parameters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPass::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus LoPassUnit::GetParameterInfo(AudioUnitScope            inScope,
                                      AudioUnitParameterID      inParameterID,
                                      AudioUnitParameterInfo    &outParameterInfo) {
    
    OSStatus result = noErr;
    
    outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable + kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
        
        switch (inParameterID) {
            case kParameter_CutoffFrequency:
                AUBase::FillInParameterName(outParameterInfo, kParamName_LoPass_Frequency, false);
                outParameterInfo.unit           = kAudioUnitParameterUnit_Hertz;
                outParameterInfo.minValue       = kMinimumValue_LoPass_Frequency;
                outParameterInfo.maxValue       = GetSampleRate() * 0.5;
                outParameterInfo.defaultValue   = kDefaultValue_LoPass_Frequency;
                outParameterInfo.flags          += kAudioUnitParameterFlag_IsHighResolution;
                outParameterInfo.flags          += kAudioUnitParameterFlag_DisplayLogarithmic;
                break;
            case kParameter_Resonance:
                AUBase::FillInParameterName(outParameterInfo, kParamName_LoPass_Resonance, false);
                outParameterInfo.unit           = kAudioUnitParameterUnit_Decibels;
                outParameterInfo.minValue       = kMinimumValue_LoPass_Resonance;
                outParameterInfo.maxValue       = kMaximumValue_LoPass_Resonance;
                outParameterInfo.defaultValue   = kDefaultValue_LoPass_Resonance;
                outParameterInfo.flags          += kAudioUnitParameterFlag_IsHighResolution;
                break;
            default:
                result = kAudioUnitErr_InvalidParameter;
                break;
        }
    } else {
        result = kAudioUnitErr_InvalidParameter;
    }
    
    return result;
}

#pragma mark ____Properties
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPass::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus LoPassUnit::GetPropertyInfo(AudioUnitPropertyID    inID,
                                     AudioUnitScope         inScope,
                                     AudioUnitElement       inElement,
                                     UInt32                 &outDataSize,
                                     Boolean                &outWritable) {
    
//    if (inScope == kAudioUnitScope_Global) {
//        
//        switch (inID) {
//            case kAudioUnitProperty_CocoaUI:
//                outWritable = false;
//                outDataSize = sizeof(AudioUnitCocoaViewInfo);
//                return noErr;
//                break;
//                
//                // Out custom property, as defined in LoPassUnit.hpp.
//            case kAudioUnitCustomProperty_FilterFrequencyResponse:
//                if (inScope != kAudioUnitScope_Global) {return kAudioUnitErr_InvalidScope;}
//                outDataSize = kNumberOfResponseFrequencies * sizeof(FrequencyResponse);
//                outWritable = false;
//                return noErr;
//        }
//    }
    
    return AUEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassUnit::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus LoPassUnit::GetProperty(AudioUnitPropertyID    inID,
                                 AudioUnitScope         inScope,
                                 AudioUnitElement       inElement,
                                 void                   *outData) {
    
    return AUEffectBase::GetProperty(inID, inScope, inElement, outData);
}

#pragma mark ___Presets
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPass::GetPresets
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/* This is used to determine if presets are supported;
 which in this audio unit they are, so we implement this method */
OSStatus LoPassUnit::GetPresets(CFArrayRef *outData) const {
    
    if (outData == NULL) {return noErr;}
    
    CFMutableArrayRef theArray = CFArrayCreateMutable(NULL, kNumberOfPresets, NULL);
    for (int i = 0; i < kNumberOfPresets; i++) {
        CFArrayAppendValue(theArray, &kPresets[i]);
    }
    
    *outData = (CFArrayRef)theArray;
    return noErr;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassUnit::NewFactoryPresetSet
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus LoPassUnit::NewFactoryPresetSet(const AUPreset &inNewFactoryPreset) {
    
    SInt32 chosenPreset = inNewFactoryPreset.presetNumber;
    
    for (int i = 0; i < kNumberOfPresets; i++) {
        
        if (chosenPreset == kPresets[i].presetNumber) {
            
            // Set the state of the parameters based on the Factory Preset selection.
            switch (chosenPreset) {
                case kPreset_Default:
                    SetParameter(kParameter_CutoffFrequency, kDefaultValue_LoPass_Frequency);
                    SetParameter(kParameter_Resonance, kDefaultValue_LoPass_Resonance);
                    break;
                case kPreset_Dark:
                    SetParameter(kParameter_CutoffFrequency, kParameter_Preset_Frequency_Dark);
                    SetParameter(kParameter_Resonance, kParameter_Preset_Resonance_Dark);
                    break;
                case kPreset_Bright:
                    SetParameter(kParameter_CutoffFrequency, kParameter_Preset_Frequency_Bright);
                    SetParameter(kParameter_Resonance, kParameter_Preset_Resonance_Bright);
                    break;
            }
            
            SetAFactoryPresetAsCurrent(kPresets[i]);
            return noErr;
        }
    }
    
    return kAudioUnitErr_InvalidPropertyValue;
}

#pragma mark ____LoPass Kernel Construction Initialiser
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassKernel::LoPassKernel()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LoPassKernel::LoPassKernel(AUEffectBase *inAudioUnit) : AUKernelBase(inAudioUnit) {
    
    Reset();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassKernel::~LoPassKernel()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LoPassKernel::~LoPassKernel() { }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassKernel::Reset()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Its very important to fully reset the filter state variables to their
 initial setting here. For delay/reverb effects, the delay buffers must
 also be cleared. */

void LoPassKernel::Reset() {
    mX1 = 0.0;
    mX2 = 0.0;
    mY1 = 0.0;
    mY2 = 0.0;
    
    // Forces filter coefficient calculation.
    mLastCutoff     = -1.0;
    mLastResonance  = -1.0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassKernel::CalculateLopassParams()
//
// inFreq is normalised frequency 0 -> 1
// inResonance is in decibels
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LoPassKernel::CalculateLopassParams(double inFreq,
                                         double inResonance) {
    
    // Convert from decibels to linear
    double r = pow(10.0, 0.05 * -inResonance);
    
    double k    = 0.5 * r * sin(M_PI * inFreq);
    double c1   = 0.5 * (1.0 - k) / (1.0 + k);
    double c2   = (0.5 + c1) * cos(M_PI * inFreq);
    double c3   = (0.5 + c1 - c2) * 0.25;
    
    mA0 = 2.0 *     c3;
    mA1 = 2.0 *     2.0 * c3;
    mA2 = 2.0 *     c3;
    mB1 = 2.0 *     -c2;
    mB2 = 2.0 *     c1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassKernel::GetFrequencyResponse()
//
// returns a scalar magnitude response
// inFreq is in Hertz.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double LoPassKernel::GetFrequencyResponse(double inFreq) {
    
    float srate = GetSampleRate();
    
    double scaledFrequency = 2.0 * inFreq / srate;
    
    // frequency on unit circle in z-plane
    double zr = cos(M_PI * scaledFrequency);
    double zi = sin(M_PI * scaledFrequency);
    
    // zeros respone
    double num_r = mA0 * (zr*zr - zi*zi) + mA1 * zr + mA2;
    double num_i = 2.0 * mA0 * zr * zi + mA1 * zi;
    
    double num_mag = sqrt(num_r * num_r + num_i * num_i);
    
    // poles response
    double den_r = zr * zr - zi * zi + mB1 * zr + mB2;
    double den_i = 2.0 * zr * zi + mB1 * zi;
    
    double den_mag = sqrt(den_r * den_r + den_i * den_i);
    
    // total response
    double response = num_mag / den_mag;
    
    return response;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPassKernel::Process()
//
// We process one non-interleaved stream at a time.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LoPassKernel::Process(const Float32    *inSourceP,
                           Float32          *inDestP,
                           UInt32           inFramesToProcess,
                           UInt32           inNumChannels, // for version 2 AudioUnits inNumChannels is always 1
                           bool             &ioSilence) {
    
    double cutoff       = GetParameter(kParameter_CutoffFrequency);
    double resonance    = GetParameter(kParameter_Resonance);
    
    // do bounds checking on parameters
    if (cutoff < kMinimumValue_LoPass_Frequency) { cutoff = kMinimumValue_LoPass_Frequency; }
    
    if (resonance < kMinimumValue_LoPass_Resonance) { resonance = kMinimumValue_LoPass_Resonance; }
    if (resonance > kMaximumValue_LoPass_Resonance) { resonance = kMaximumValue_LoPass_Resonance; }
    
    // Convert to 0->1 normalized frequency
    float srate = GetSampleRate();
    
    cutoff = 2.0 * cutoff / srate;
    if (cutoff > 0.99) { cutoff = 0.99; } // clip cutoff to highest allowed by sample rate.
    
    
    // only calculate the filter coefficients if the parameters have changed from last time
    if (cutoff != mLastCutoff || resonance != mLastResonance) {
        
        CalculateLopassParams(cutoff, resonance);
        
        mLastCutoff = cutoff;
        mLastResonance = resonance;
    }
    
    const Float32 *sourceP  = inSourceP;
    Float32 *destP          = inDestP;
    int n                   = inFramesToProcess;
    
    //Apply the filter on the input and write to the ouput
    
    while (n--) {
        
        float input = *sourceP++;
        float output = mA0*input + mA1*mX1 + mA2*mX2 - mB1*mY1 - mB2* mY2;
        
        mX2 = mX1;
        mX1 = input;
        mY2 = mY1;
        mY1 = output;
        
        *destP++ = output;
    }
}

