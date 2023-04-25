#include "rms.hpp"

// float rms::calc_irms(void)//uint8_t channel)//, uint8_t numberOfCycles)
// {
// 	int i, j=0;
// 	uint8_t high, low;
// 	int divScale_count = 1;

// 	// ADC converter
// 	const float f = 60.0;									// Hertz;
// 	const int numberOfCycles = 16;							// Number of cycles;
// 	const int divScale = 8;									// Prescale for real sample rate Fs;

// 	const float Fs = 16000000/128/13;									// Sample rate of signal processed;
// 	const int nPointsPerCycle = (int) Fs/f;								// Number of points per cycle;
// 	const int nPoints = (int) nPointsPerCycle*numberOfCycles; 			// Number of signal points.

// 	const float Fs_div = 16000000/128/13/divScale;						// Sample rate of signal processed;
// 	const int nPointsPerCycle_div = (int) Fs_div/f;						// Number of points per cycle;
// 	const int nPoints_div = (int) nPointsPerCycle_div*numberOfCycles;	// Number of signal points.

// 	int *adcSamples = NULL;
// 	adcSamples = (int*)malloc(nPoints_div * sizeof(int));

// 	// 160.2564 = 16000000/128/13/60.0;
// 	for(i=0;i<nPoints;i++)
// 	{
// 		ADCSRA |= (1<<ADSC);				// Start conversion;
// 		while (bit_is_set(ADCSRA, ADSC));	// wait until conversion done;

// //		Serial.println((ADCH << 8) | ADCL);

// 		if(divScale_count == 1)
// 		{
// 			low  = ADCL;
// 			high = ADCH;

// 			j = (int) i/divScale;
// 			adcSamples[j] = (high << 8) | low;
// 			divScale_count = divScale;
// 		}
// 		else
// 		{
// 			divScale_count--;
// 		}
// 	}

// 	float *vs = NULL;
// 	vs = (float*)malloc(nPoints_div * sizeof(float));

// 	for(i=0;i<nPoints_div;i++)
// 	{
// 		vs[i] = (adcSamples[i]*5.0)/1024.0;
// 	}

// 	free(adcSamples);

// 	// Offset remove.
// 	float Vmean = 0.0;
// 	for(i=0;i<nPoints_div;i++)
// 		Vmean += vs[i];

// 	Vmean = Vmean/nPoints_div;

// 	for(i=0;i<nPoints_div;i++)
// 		vs[i] = vs[i] - Vmean;

// 	float *vs2 = NULL;
// 	vs2 = (float*)malloc(nPoints_div * sizeof(float));

// 	// Power signal
// 	for(i=0;i<nPoints_div;i++)
// 		vs2[i] = vs[i]*vs[i];

// 	free(vs);

// 	float sum=0;
// 	float V2mean;

// 	// mean finder
// 	for(i=0;i<nPoints_div;i++)
// 		sum += vs2[i];
// 	V2mean = sum/nPoints_div;

// 	free(vs2);

// 	float I = 0.0;
// 	float k = 2020.0;
// 	float R = 310.0;

// 	// RMS equation
// 	I = (k*sqrt(V2mean))/R;

// 	return I;
// }