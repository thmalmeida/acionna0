#ifndef RMS_HPP__
#define RMS_HPP__

#include <stdio.h>
#include <math.h>

/* Root Mean Square - RMS class
	Xi: each value
	n: number o measurements samples;
	
	RMS = sqrt(sum(Xi)^2)/n_samples)	
*/
// uint16_t v2_k[466] = {2048, 2083, 2119, 2154, 2189, 2225, 2260, 2295, 2330, 2364, 2399, 2433, 2467, 2500, 2534, 2567, 2599, 2631, 2663, 2694, 2725, 2755, 2785, 2814, 2843, 2871, 2898, 2925, 2951, 2977, 3001, 3026, 3049, 3072, 3094, 3115, 3135, 3155, 3174, 3192, 3209, 3225, 3241, 3255, 3269, 3282, 3294, 3305, 3315, 3324, 3332, 3339, 3346, 3351, 3356, 3359, 3362, 3363, 3364, 3363, 3362, 3360, 3356, 3352, 3347, 3341, 3334, 3326, 3317, 3307, 3296, 3285, 3272, 3259, 3244, 3229, 3213, 3196, 3178, 3160, 3140, 3120, 3099, 3077, 3055, 3032, 3008, 2983, 2958, 2931, 2905, 2878, 2850, 2821, 2792, 2762, 2732, 2702, 2671, 2639, 2607, 2575, 2542, 2509, 2475, 2442, 2407, 2373, 2339, 2304, 2269, 2234, 2198, 2163, 2127, 2092, 2056, 2021, 1985, 1950, 1914, 1879, 1844, 1809, 1774, 1739, 1705, 1670, 1637, 1603, 1570, 1537, 1504, 1472, 1440, 1409, 1378, 1348, 1318, 1288, 1260, 1231, 1204, 1177, 1150, 1125, 1100, 1075, 1052, 1029, 1007, 985, 965, 945, 926, 908, 890, 874, 858, 843, 829, 816, 804, 793, 783, 773, 765, 757, 751, 745, 741, 737, 734, 732, 731, 732, 733, 735, 738, 742, 746, 752, 759, 767, 776, 785, 796, 807, 820, 833, 847, 862, 878, 895, 912, 931, 950, 970, 991, 1012, 1034, 1058, 1081, 1106, 1131, 1157, 1183, 1211, 1238, 1267, 1296, 1325, 1355, 1386, 1417, 1448, 1480, 1512, 1545, 1578, 1611, 1645, 1679, 1713, 1748, 1783, 1817, 1853, 1888, 1923, 1959, 1994, 2030, 2065, 2101, 2136, 2172, 2207, 2242, 2278, 2312, 2347, 2382, 2416, 2450, 2484, 2517, 2550, 2583, 2615, 2647, 2678, 2709, 2740, 2770, 2799, 2828, 2857, 2884, 2912, 2938, 2964, 2989, 3014, 3037, 3061, 3083, 3104, 3125, 3145, 3164, 3183, 3200, 3217, 3233, 3248, 3262, 3275, 3288, 3299, 3310, 3319, 3328, 3336, 3343, 3349, 3353, 3357, 3360, 3362, 3363, 3364, 3363, 3361, 3358, 3354, 3350, 3344, 3338, 3330, 3322, 3312, 3302, 3291, 3279, 3266, 3252, 3237, 3221, 3205, 3187, 3169, 3150, 3130, 3110, 3088, 3066, 3043, 3020, 2995, 2970, 2945, 2918, 2891, 2864, 2835, 2807, 2777, 2747, 2717, 2686, 2655, 2623, 2591, 2558, 2525, 2492, 2458, 2425, 2390, 2356, 2321, 2286, 2251, 2216, 2181, 2145, 2110, 2074, 2039, 2003, 1968, 1932, 1897, 1861, 1826, 1791, 1756, 1722, 1688, 1653, 1620, 1586, 1553, 1520, 1488, 1456, 1424, 1393, 1363, 1333, 1303, 1274, 1245, 1217, 1190, 1164, 1137, 1112, 1087, 1063, 1040, 1018, 996, 975, 955, 935, 917, 899, 882, 866, 851, 836, 823, 810, 799, 788, 778, 769, 761, 754, 748, 743, 739, 735, 733, 732, 731, 732, 733, 736, 739, 744, 749, 756, 763, 771, 780, 790, 801, 813, 826, 840, 854, 870, 886, 903, 921, 940, 960, 980, 1001, 1023, 1046, 1069, 1094, 1118, 1144, 1170, 1197, 1224, 1252, 1281, 1310, 1340, 1370, 1401, 1432, 1464, 1496, 1528, 1561, 1595, 1628, 1662, 1696, 1731, 1765, 1800, 1835, 1870, 1906, 1941, 1976, 2012, 2047};

class rms {
public:
	// Circuit polarization parameters for currente sensor
	double Vmax = 2.450;							// Maximum voltage ADC can read [V];
	double Vmin = 0.120;							// Minimum voltage ADC can read [V];                     
	int m_bits = 2895;                              // Max decimal value correlated with Vmax on 12 bit range;
	double GND  = 0.0;								// gnd for ADC converter [V];
	double Vdc = 3.3;								// Voltage divisor supply [V];
	double R1 = 56*1000;							// Voltage divisor top resistor [Ohms];
	double R2 = 40*1000;							// Voltage divisor bottom resistor [Ohms];
	double V_R2 = R2/(R1+R2)*Vdc;					// Voltage over R2 [V] or Vref for ADC converter [V];

	double Rb1 = 220.0;								// Burden resistor (bias) [Ohms];
	double Rb2 = 75.0;								// Burden resistor (bias) [Ohms];
	double N1 = 1;									// Current transformer sensor ration parameters
	double N2 = 2000;								// Current transformer sensor ration parameters

	// ADC sampling parameters. F_clk MHz, internal div = 128, 13 cycles for ADC conversion.
	unsigned int n_bits = 12;						// ADC conversion resolution;
	int F_clk = 8e6;								// Crystal system clock [Hz];
	int div_1 = 1;									// AHB bus prescale;
	int div_2 = 1;									// APB2 bus prescale;
	int div_3 = 8;									// ADC prescale;

	double adc_clk = F_clk/div_1/div_2/div_3;		// ADC clock after all prescalers
	double adc_sampling_time = 1.5;					// amount of time clock to sample [cycles];
	double adc_conv = 12.5;							// Fixed number of ADC peripheral takes to convert [cycles];
	double T_conv = adc_sampling_time + adc_conv; 	// ADC conversion time [cycles];

	int n_samples;				// number of samples (points)
	int n_cycles;				// number of waves cycles
	int f;						// signal frequency [Hz]
	int nb;						// resolution [number of bits]
	int Fs;						// sample frequency	[samples/s]

	rms() {
		// calc_k_();
	}

	// template <typename T> T calc_rms(T* x, int n)
	// {
	// 	double sum = 0;

	// 	for (int i = 0; i < n; i++) {
	// 	sum += pow(x[i], 2);
	// 	printf("x[%d]: %f\n", i, x[i]);
	// 	}
	// 	return sqrt(sum / n);
	// }
	double calc_rms(double *v, int len) {
		double sum = 0;

		for(int i=0; i<len; i++) {
		sum += pow(v[i], 2);
		// printf("%f \n", v[i]);
		}
		return sqrt(sum/len);
	}

	void calc_Vadc_t(uint16_t* d_out, double* Vadc_t, int length) {
		// Finding Vadc_t after ADC read
		for(int i=0; i<length; i++) {
			Vadc_t[i] = d_out[i]*(Vmax-Vmin)/(m_bits) + Vmin;
		}
    }

	void calc_iL_t(uint16_t* d_out, double* iL_t, int length) {
		for(int i=0; i<length; i++) {
			iL_t[i] = (d_out[i]*(Vmax-Vmin)/(m_bits) + Vmin - V_R2)*(1/Rb2)*(N2/N1);
		}
	}

	void filter(uint16_t* v, int length) {
		for(int i=1; i<length; i++) {
			v[i] = 0.8*v[i-1] + 0.2*v[i];
		}
	}

    // i1_t = (Vdc*v2_k/(2^n_bits-1) - R2/(R1+R2)*Vdc)*(1/Rb)*(N2/N1);
    // i1_t = (v2_k*k1_ - k2_)*k3_
    // i1_t = v2_k*k1_*k3_ - k2_*k3_

    // v2_k vector is the digital value converted
    // convert v2_k digital read to i1_t load current based on constants
    // void convert_v2k_to_i1t(uint16_t *v2_k, double* i1_t, int len) {
    //     // double i1_t[len];
    //     for(int i=0; i<len; i++) {
    //         i1_t[i] = static_cast<double>(v2_k[i])*k1k3_ - k2k3_;
    //         // printf("%f \n", i1_t[i]);
    //     }
    //     // double *p = &i1_t[0];
    //     // return p;
    // }
    // void calc_k1_(void) {
    //     k1_ = Vdc/(pow(2, n_bits)-1);
    //     printf("k1_: %f\n", k1_);
    // }

    // void calc_k2_(void) {
    //     k2_ = R2/(R1+R2)*Vdc;
    //     printf("k2_: %f\n", k2_);
    // }

    // void calc_k3_(void) {
    //     k3_ = (1/Rb)*(N2/N1);
    //     printf("k3_: %f\n", k3_);
    // }

    // void calc_k1k3_(void) {
    //     k1k3_ = k1_*k3_;
    //     printf("k1k3_: %f\n", k1k3_);
    // }

    // void calc_k2k3_(void) {
    //     k2k3_ = k2_*k3_;
    //     printf("k2k3_: %f\n", k2k3_);
    // }
    
    // void calc_k_(void) {
    //     calc_k1_();
    //     calc_k2_();
    //     calc_k3_();
    //     calc_k1k3_();
    //     calc_k2k3_();
    // }

    // private:
    double i1_t[466];
    double k1_;
    double k2_;
    double k3_;
    double k1k3_;
    double k2k3_;
};

#endif


// // RMS calculation example
// //Reference: https://rosettacode.org/wiki/Averages/Root_mean_square
// C example:
// #include <stdio.h>
// #include <math.h>
// double rms(double *v, int n)
// {
// 	int i;
// 	double sum = 0.0;
// 	for(i = 0; i < n; i++)
// 		sum += v[i] * v[i];
// 	return sqrt(sum / n);
// }
// int main(void)
// {
// 	double v[] = {1., 2., 3., 4., 5., 6., 7., 8., 9., 10.};
// 	printf("%f\n", rms(v, sizeof(v)/sizeof(double)));
// 	return 0;
// }
// // C++ example
// #include <iostream>
// #include <vector>
// #include <cmath>
// #include <numeric>
// int main(void) {
// 	std::vector<int> numbers ;
// 	for ( int i = 1 ; i < 11 ; i++ ) {
// 		numbers.push_back( i );
// 	}
// 	double meansquare = sqrt( ( std::inner_product( numbers.begin(), numbers.end(), numbers.begin(), 0 ) ) / static_cast<double>( numbers.size() ) );
// 	std::cout << "The quadratic mean of the numbers 1 .. " << numbers.size() << " is " << meansquare << " !\n" ;
// 	return 0 ;
// }


// float calcIrms()//uint8_t channel)//, uint8_t numberOfCycles)
// {
// 	int i, j=0;
// 	uint8_t high, low;
// 	int divScale_count = 1;

// 	ADCSRB &= ~(1<<MUX5);
// 	ADMUX  &= ~(1<<MUX4);
// 	ADMUX  &= ~(1<<MUX3);
// 	ADMUX  &= ~(1<<MUX2);
// 	ADMUX  &= ~(1<<MUX1);
// 	ADMUX  |=  (1<<MUX0);									// Select ADC1

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


// //	sprintf(buffer,"---- Signal Captured ----");
// //	Serial1.println(buffer);
// //	Serial1.println("");
// //
// //	Serial1.print("Fs:");
// //	Serial1.println(Fs);
// //	Serial1.println("");
// //
// //	sprintf(buffer,", nPointsPerCycle:%d", nPointsPerCycle);
// //	Serial1.println(buffer);
// //	Serial1.println("");
// //
// //	sprintf(buffer,"nPoints:%d", nPoints);
// //	Serial1.println(buffer);
// //	Serial1.println("");
// //
// //
// //
// //	sprintf(buffer,"---- Signal Processed ----");
// //	Serial1.println(buffer);
// //	Serial1.println("");
// //
// //	Serial1.print("Fs:");
// //	Serial1.println(Fs_div);
// //	Serial1.println("");
// //
// //	sprintf(buffer,", nPointsPerCycle:%d", nPointsPerCycle_div);
// //	Serial1.println(buffer);
// //	Serial1.println("");
// //
// //	sprintf(buffer,"nPoints:%d", nPoints_div);
// //	Serial1.println(buffer);
// //	Serial1.println("");


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

// //	Serial.println("ENTROU!");
// //	for(i=0;i<nPoints_div;i++)
// //	{
// //		Serial.println(adcSamples[i]);
// //	}
// //	Serial.println("SAIU!");


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