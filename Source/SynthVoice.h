/*
  ==============================================================================

    SynthVoice.h
    Created: 22 Apr 2021 9:05:02am
    Author:  Alberto

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"

using namespace juce;

class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float veloc, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    void setInitialConditions(float veloc, double freq);
    void updateParams(const float tension, const float sustain);
    void renderNextBlock(juce::AudioBuffer <float>& outputBuffer, int startSample, int numSamples) override;
    
    std::vector<float> getVisual();
    int getNumCuerda();
    int getNumTraste();
    
private:
    float xi(float w);
    
    int   numCuerda;                                // Número de cuerda que se está tocando
    int   numTraste;

    float dt;                                       // Periodo de muestreo
    float dx;                                       // Distancia de muestreo espacial
    int   X;                                        // Longitud de la cuerda en pasos de muestreo L = X * dx;
    int   xRead;                                    // Posición de lectura de la cuerda (0 - 1)
    
    float s0;                                       // Parámetros de atenuación
    float s1;                                       //

    float k;                                        // Parámetros de estabilidad
    float gamma;                                    // 

    float c0;                                       // Velocidad inicial de la cuerda
    float c;                                        // Velocidad de propagación en la cuerda
    
    float tMult;                                    // Multiplicador de la tensión de la cuerda (controlado por el usuario)
    float decMult;
    //float r;                                        // Radio de la cuerda (m)
    //float E;                                        // Young's modulus (stiffness)
    //float S;                                        // Sección de la cuerda
    //float K;                                        // r/2 (stiffness)

    float timeDet = 1.0f;                           // Tiempo de reacción del detector (ms)
    float alfa;                                     // Parámetro para el detector de nivel
    float c2n;                                      // Valor del detector de nivel RMS

    // Se establecen las características de las 16 cuerdas (características medidas/calculadas usando cuerdas reales)

    float L_esc = 1;                           // Longitud de escala 27" = 68.58 cm

    float Strings[2][16] = {
        // {   89.7114f, 100.6977f, 113.0293f, 126.8711f, 142.4077f, 159.8476f, 179.8476f, 201.3948f, 226.0589f, 253.7419f, 284.8155f, 319.6953f, 358.8462f, 402.7907f, 452.1178f, 507.4838f },    // Velocidad del sonido en la cuerda
        // {  1.626e-3f, 1.422e-3f, 1.219e-3f, 1.016e-3f, 8.636e-4f,  7.62e-4f, 6.604e-4f, 5.588e-4f, 4.572e-4f, 4.064e-4f, 3.556e-4f, 3.048e-4f, 3.048e-4f,  2.54e-4f,  2.29e-4f,  2.03e-4f },    // Diámetro
        {     65.4f,    73.4f,    82.4f,    92.5f,   103.8f,   116.5f,  130.8f,   146.8f,   164.8f,   185.0f,   207.6f,   233.0f,   261.6f,   293.6f,   329.6f,   369.9f },    // Frecuencia min
        {    185.0f,   207.5f,   233.0f,   261.6f,   293.6f,   329.6f,  370.0f,   415.3f,   466.1f,   523.2f,   587.3f,   659.2f,   740.0f,   830.6f,   932.3f,  1046.5f }     // Frecuencia max
    };

    float loss[2][2] = {
        { 200 * 2 * float_Pi , 10000 * 2 * float_Pi},
        {    9              ,            6       }          // Frecuencias y tiempos de caída por pérdidas
    };
    
    int   pos = 1;                                  // Posición en la que se toca la cuerda (Posición 1: trastes 1-6, posición 2: trastes 7-13...)
    float ctr = 0.6;                               // Punto de máxima velocidad inicial
    float read = 0.8f;                              // Posición de lectura de la cuerda 0-1 (0.7 Ok)
    float lambda = 1;                               // Estabilidad ¡¡Ejemplo lambda = 1!!

    std::vector<float> v0;
    std::vector<float> yNext;
    std::vector<float> yPrev;
    std::vector<float> y;

    std::vector<float> visualCuerda;

    juce::AudioBuffer<float> synthBuffer;

    bool isPrepared = false;
};