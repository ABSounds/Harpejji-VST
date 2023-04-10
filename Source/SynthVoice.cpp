/*
  ==============================================================================

    SynthVoice.cpp
    Created: 22 Apr 2021 9:05:02am
    Author:  Alberto Barrera Herrero

  ==============================================================================
*/

#include "SynthVoice.h"
#include <math.h>


bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound) {
    return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr;   // Se comprueba que el sonido cargado es un objeto SynthesiserSound válido
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) {
    if (juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber) > 65.40f && juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber) < 1047)
        setInitialConditions(velocity, juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    else
        clearCurrentNote();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff) {
    s0 = 200 * s0;
}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue) {

}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue) {

}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels) {
    alfa = (1000.0f / timeDet) * (1.0f / getSampleRate());
    dt = 1.0f / (float)getSampleRate();           // Periodo de muestreo
    isPrepared = true;
}

void SynthVoice::setInitialConditions(float velocity, double frequency) {    // Velocity y Frequency valor
    gamma = 2.0f * frequency;
    dx = gamma * dt / lambda;                     // Paso de muestreo espacial

    numCuerda = 0;

    while (frequency > (Strings[0][numCuerda] * powf(2, 3.0f * pos / 12.0f)) && numCuerda < 15) {
        numCuerda++;
    }

    numTraste = (int) round(12.0f * log2f(frequency / Strings[0][numCuerda])) + 1;

    c0 = 2 * L_esc * Strings[0][numCuerda];
    c = c0 * tMult;

    //r =  Strings [1] [i] / 2;
    //S = r * juce::float_Pi * juce::float_Pi;        // Área de la sección de la cuerda
    //K = r / 2.0f;

    // Longitud de la cuerda
    float L = c / (2.0f * frequency);                   // Longitud de la cuerda en metros
    X = (int)floor(L / dx);                             // Longitud de la cuerda en número de pasos dx
    dx = L / X;                                         //

    k = sqrtf(0.001f) * (gamma / juce::float_Pi);       // Estabilidad

    int xCtr = (int)floor(X * ctr);
    xRead = (int)floor(X * read);

    v0 = std::vector<float>((X + 1), 0);                //
    yPrev = v0;                                         // Se inicializan a 0
    y = v0;                                             //
    yNext = v0;                                         //

    // Coeficiente de atenuación lineal
    s0 = decMult * (xi(loss[0][1]) / loss[1][0] - xi(loss[0][0]) / loss[1][1]) * 6.0 * logf(10) / (xi(loss[0][1]) - xi(loss[0][0]));
    
    // Coeficiente de atenuación dependiente de la frecuencia
    s1 = decMult * (-1 / loss[1][0] + 1 / loss[1][1]) * 6 * log(10) / (xi(loss[0][1]) - xi(loss[0][0]));

    // Se establecen las condiciones iniciales en la cuerda -> Velocidad inicial en en la cuerda tras ser pulsada.

    // Se inicializa el detector de nivel a 1 para que no se apague inmediatamente la voz
    c2n = 1;

    // Tipo de excitación
    int excitacion = 4;

    switch (excitacion) {
        case 1:     // Rampa
            for (int x = 1; x < X; x++) {
                v0[x] = x / X;
            }
        break;

        case 2:     // Raised cosine modificado
            for (int x = 0; x <= X; x++) {
                if (x <= xCtr)
                    v0[x] = (0.5f - 0.5f * cos(((float)x / (float)xCtr) * juce::float_Pi));
                else
                    v0[x] = (0.5f + 0.5f * cos((float)(x - xCtr) * juce::float_Pi / (float)(X - xCtr)));
            }
            break;

        case 3:     // Misma velocidad en toda la cuerda
            for (int x = 1; x < X; x++) {
                v0[x] = 1;
            }
            break;

        case 4:     // Trayectoria de proyectil.
            float t1;
            float t2;
            for (int x = 1; x < X; x++) {
                // Forma para velocity altas -> sonido más percusivo
                t1 = -10.0f * logf(1.0f - 30.0f * ((float)x / X) / 92.388f);
                v0[X-x] =  velocity * (-t1 + (1 - expf(-10.0f * t1)) * 3.92683f) / 3.4597f;

                // forma para velocity baja -> sonido más suave
                t2 = -3.0f * log(1.0f - 0.74936f * ((float)x / X));
                v0[X-x] = v0[X - x] +  (1 - velocity) * (-1.6667f * t2 + (1.0f - exp(-(3.0f * t2))) * 6.8964f) / 4.9411f;
            }
            break;
    }

    float kn = 2 * float_Pi * frequency / c;                        // Cálculo del número de onda k
    float integral = 0;

    for (int x = 0; x < X; x++) {                                   //
        integral = integral + v0[x] * sin(kn * x * dx) * dx;        //  Se calcula la amplitud que tendrá la cuerda excitada 
    }                                                               //  para normalizarla y que todas las notas tengan el mismo volumen
                                                                    
    float b_n = (2 * integral / (L * 2 * float_Pi * frequency));    // b_n = amplitud de la vibración generada. Para normalizar se divide entre este valor

    for (int x = 0; x < X; x++) {                                   //  Normalización del volumen
        v0[x] = velocity * v0[x] / b_n;                             // 
    }

    for (int x = 0; x < X; x++)                                     // Se calcula la posición de la cuerda en el instante siguiente
        y[x] = v0[x] * dt;
}

void SynthVoice::updateParams(const float tension, const float sustain) {
    tMult = tension;
    decMult = sustain;
}

void SynthVoice::renderNextBlock(juce::AudioBuffer <float>& outputBuffer, int startSample, int numSamples) {
    jassert(isPrepared);    // Se comprueba que se ha llamado a la función prepareToPlay, si no, se detiene la ejecución
    
    if (!isVoiceActive())
        return;

    // float stiffness;

    synthBuffer.setSize(1, numSamples, false, false, true);
    synthBuffer.clear();

    // Cálculo del término de la ecuación diferencial asociado a la rigidez
    for (int s = 0; s < synthBuffer.getNumSamples(); s++) {
        for (int x = 1; x < X-1; x++) {
            //if (x < 2)
            //    stiffness = E * S * powf(K, 2) * (y[x + 2] - 4.0f * y[x + 1] + 6.0f * y[x] - 4.0f * y[x - 1]) * (powf(dt, 2) / powf(dx, 4));                  // Forward finite differences
            //else{
            //    if (x > X - 2)
            //        stiffness = E * S * powf(K, 2) * (-4.0f * y[x + 1] + 6.0f * y[x] - 4.0f * y[x - 1] + y[x - 2]) / (powf(dt, 2) / powf(dx, 4));             // Backwards finite differences
            //    else
            //        stiffness = E * S * powf(K, 2) * (y[x + 2] - 4.0f * y[x + 1] + 6.0f * y[x] - 4.0f * y[x - 1] + y[x - 2]) * (powf(dt, 2) / powf(dx, 4));   // Central finite differences
            //}
            
            // Cálculo de la posición de la cuerda en el sample siguiente
            yNext[x] = powf(c, 2) * (y[x - 1] - 2.0f * y[x] + y[x + 1]) * (powf(dt, 2) / powf(dx, 2)) + 2 * y[x] - yPrev[x]                         //String Wave Equation
                - 2.0f * s0 * (y[x] - yPrev[x]) * dt                                                                                                // Linear Damping
                + 2.0f * s1 * (y[x + 1] - 2.0f * y[x] + y[x - 1] - yPrev[x + 1] + 2.0f * yPrev[x] - yPrev[x - 1]) * dt / powf(dx, 2);                // Freq. dependant damping
                //- stiffness;                                                                                                                        // Stiffness
        }

        synthBuffer.addSample(0, s, y[xRead]);
        
        c2n = alfa * powf(y[xRead], 2) + (1 - alfa) * c2n;
        if (c2n < 0.000000005f) { 
            numTraste = -1;
            numCuerda = -1;
            visualCuerda.clear();
            clearCurrentNote();
            return;
        }

        // Se pasa la posición de la cuerda al UI
        if (s % 200 == 0)
            visualCuerda = y;

        // Se guarda la posición de la cuerda actual y se actualiza
        yPrev = y;
        y = yNext;
    }

    // Se copian los samples del buffer de la voz al buffer de salida
    for (int channel = 0; channel < outputBuffer.getNumChannels(); channel++) {
        outputBuffer.addFrom(channel, startSample, synthBuffer, 0, 0, numSamples);
    }
}

// Letra griega xi

float SynthVoice::xi(float w) {
    double result;
    
    result = (-pow((double)gamma, 2) + sqrt (pow((double)gamma, 4) + 4 * pow((double)k, 2) * pow((double)w, 2))) / (2 * pow((double)k, 2));

    return (float) result;
}

std::vector<float> SynthVoice::getVisual() 
{
    return visualCuerda;
}

int SynthVoice::getNumCuerda()
{
    return numCuerda;
}

int SynthVoice::getNumTraste()
{
    return numTraste;
}