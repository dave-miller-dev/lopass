//
//  LoPassUnit.hpp
//  LoPass
//
//  Created by David Miller on 28/9/21.
//

#include "AUEffectBase.h"
#include "LoPassVersion.h"

#if AU_DEBUG_DISPATCHER
    #include "AUDebugDispatcher.h"
#endif

#ifndef LoPassUnit_hpp
#define LoPassUnit_hpp

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Custom Property
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define kNumberOfResponseFrequencies 512

// Here we define a custom property so the view is able to retrieve the current frequency
// response curve.  The curve changes as the filter's cutoff frequency and resonance are
// changed...

// custom properties id's must be 64000 or greater
// see <AudioUnit/AudioUnitProperties.h> for a list of Apple-defined standard properties
//
//enum {
//    kAudioUnitCustomProperty_FilterFrequencyResponse    = 65536
//};

// We'll define our property data to be a size kNumberOfResponseFrequencies array of structs
// The UI will pass in the desired frequency in the mFrequency field, and the Filter AU
// will provide the linear magnitude response of the filter in the mMagnitude field
// for each element in the array.

//typedef struct FrequencyResponse {
//    Float64 mFrequency;
//    Float64 mMagnitude;
//} FrequencyResponse;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parameters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____Parameters

// Define constants to represent the User Interface parameter name, min/max and default values.
static CFStringRef      kParamName_LoPass_Frequency     = CFSTR("cutoff frequency");
static constexpr float  kMinimumValue_LoPass_Frequency  = 12.0;
//static constexpr float  kMaximumValue_LoPass_Frequency  = 20000.0;
static constexpr float  kDefaultValue_LoPass_Frequency  = 1000.0;

static CFStringRef      kParamName_LoPass_Resonance     = CFSTR("resonance");
static constexpr float  kMinimumValue_LoPass_Resonance  = -20.0;
static constexpr float  kMaximumValue_LoPass_Resonance  = 20.0;
static constexpr float  kDefaultValue_LoPass_Resonance  = 0.0;

// Define an enum to represent ParameterID values.
enum Parameters {
    kParameter_CutoffFrequency          = 0,
    kParameter_Resonance                = 1,
    kNumberOfParameters                 = 2
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Factory Presets
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____Factory Presets

enum FactoryPresets {
    kPreset_Default     = 0,
    kPreset_Dark        = 1,
    kPreset_Bright      = 2,
    kNumberOfPresets    = 3,
};

static constexpr float kParameter_Preset_Frequency_Default  = 1000.0;
static constexpr float kParameter_Preset_Resonance_Default  = 0.0;

/// Define a constant for the Cutoff Freq for preset "dark".
static constexpr float kParameter_Preset_Frequency_Dark     = 200.0;
/// Define a constant for the Resonance for preset "dark".
static constexpr float kParameter_Preset_Resonance_Dark     = -5.0;
/// Define a constant for the Cutoff Freq for preset "bright".
static constexpr float kParameter_Preset_Frequency_Bright   = 1000.0;
/// Define a constant for the Resonance for preset "bright".
static constexpr float kParameter_Preset_Resonance_Bright   = 10.0;

static AUPreset kPresets[kNumberOfPresets] = {
    { kPreset_Default, CFSTR("Default") },
    { kPreset_Dark, CFSTR("Dark") },
    { kPreset_Bright, CFSTR("Bright") }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPass DSP Kernel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____DSP Kernel

/// Actual DSP Filtering happens here.
class LoPassKernel: public AUKernelBase {
    
public:
    LoPassKernel(AUEffectBase *inAudioUnit);
    
    virtual ~LoPassKernel();
    
    virtual void Process(const Float32  *inSourceP,
                         Float32        *inDestP,
                         UInt32         inFramesToProcess,
                         UInt32         inNumChannels,
                         bool           &ioSilence);
    
    /// Reset the filter state.
    virtual void Reset();
    
    void CalculateLopassParams(double inFreq, double inResonance);
    
    double GetFrequencyResponse(double inFreq);
    
private:
    // Filter coefficients
    double mA0;
    double mA1;
    double mA2;
    double mB1;
    double mB2;
    
    // Filter state
    double mX1;
    double mX2;
    double mY1;
    double mY2;
    
    double mLastCutoff;
    double mLastResonance;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LoPass class
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____LoPass Unit
class LoPassUnit : public AUEffectBase {
    
public:
    LoPassUnit(AudioUnit component);
    
#if AU_DEBUG_DISPATCHER
    virtual ~LoPassUnit () { delete mDebugDispatcher };
#endif
    
    /// Provide the audio unit version information.
    virtual OSStatus Version() { return kLoPassVersion; }
    
//    virtual OSStatus Initialize();
    
    virtual AUKernelBase* NewKernel() { return new LoPassKernel(this); }
    
    // For custom property
    virtual OSStatus    GetPropertyInfo(    AudioUnitPropertyID    inID,
                                            AudioUnitScope         inScope,
                                            AudioUnitElement       inElement,
                                            UInt32                 &outDataSize,
                                            Boolean                &outWritable);
    
    virtual OSStatus    GetProperty(        AudioUnitPropertyID        inID,
                                            AudioUnitScope             inScope,
                                            AudioUnitElement           inElement,
                                            void *                     outData);
    
    virtual OSStatus    GetParameterInfo(   AudioUnitScope            inScope,
                                            AudioUnitParameterID      inParameterID,
                                            AudioUnitParameterInfo    &outParameterInfo);
    
    // Handle Factory Presets
    virtual OSStatus    GetPresets(CFArrayRef *outData) const;
    virtual OSStatus    NewFactoryPresetSet( const AUPreset &inNewFactoryPreset);
    
    /// Support a 1ms tail time. A reverb effect would support a more substantial tail in the order of a few seconds.
    virtual bool        SupportsTail() { return true; }
    virtual Float64     GetTailTime() {return 0.001; }
    
    /// No latency.
    /// A lookahead compressor or FFT-based processor should report the true latency in seconds.
    virtual Float64 GetLatency() { return 0.0; }
    
protected:
    
};




#endif /* LoPassUnit_hpp */
