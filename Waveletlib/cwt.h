#ifndef CWT_H
#define CWT_H

#include <cstring>
#include <stdint.h>
#include <math.h>
#include "fftw3.h"

#define MEX_H_AMPL			  0.8673251

enum cwt_len_type {FULL, FIT, REDUCED};

class TCwt
{
	private:
		size_t slen;							// Signal length of each channel
		size_t wlen;                            // Wavelet length

		size_t cnv_len;
		size_t cnv_pw2_len;
		size_t cnv_red_len;
		size_t sp_len;

		size_t cwt_list_rows;

		float wscale_start;						// Wavelet scaling parameters
		float wscale_stop;
		float wscale_step;

		float *wave_temp;

		float *wsclist;							// Scaled wavelet list
		fftwf_complex *wsplist;					// Scaled wavelet spectrum

		float *s; 								// Both signals
		fftwf_complex *ssp;

		fftwf_complex *xps;						// Cross product spectrum
		float *xp;								// Cross product

		float fft_norm_factor;

		fftwf_plan wp;							// Wavelet FFT plan
		fftwf_plan sp;							// Both channels FFT plan
		fftwf_plan sgl_sp;						// Single channel only FFT plan
		fftwf_plan iwsp;						// Both channels IFFT plan
		fftwf_plan sgl_iwsp;                    // Single channel only IFFT plan

		float **ch1_out_full;
		float **ch2_out_full;

		float **ch1_out_fit;
		float **ch2_out_fit;

		float **ch1_out_rdc;
		float **ch2_out_rdc;

		bool IsPow2(size_t val);
		float *MexH(size_t len, float a);

	public:
		TCwt(size_t sl, size_t wl, float wstart, float wstop, float wstep);
		~TCwt();
		void ExecuteBothChCwt(const float *s1, const float *s2);
		float **ExecuteSingleChCwt(const float *sig, cwt_len_type wlt);
		float **GetCh1CwtResult(cwt_len_type wlt);
		float **GetCh2CwtResult(cwt_len_type wlt);
		size_t GetFullLen(void);
		size_t GetFitLen(void);
		size_t GetReducedLen(void);
		size_t GetCWTRowCnt(void);
};
#endif

