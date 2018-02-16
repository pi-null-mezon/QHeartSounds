#include "cwt.h"

TCwt::TCwt(size_t sl, size_t wl, float wstart, float wstop, float wstep)
{
	float tmp;

	slen = sl;
	wlen = wl;

	wscale_start = wstart;
	wscale_stop = wstop;
	wscale_step = wstep;

	cnv_len = slen + wlen - 1;
	cnv_red_len = slen - wlen + 1;
	cnv_pw2_len = cnv_len;
	while(IsPow2(cnv_pw2_len) == false)
		cnv_pw2_len++;
	sp_len = (cnv_pw2_len / 2) + 1;

	tmp = (wstop - wstart) / wstep;
	cwt_list_rows = ((size_t)tmp) + 1;

	fft_norm_factor = (float)cnv_pw2_len;

	wave_temp = NULL;

	wsclist = (float*)fftwf_malloc(sizeof(float) * cwt_list_rows * cnv_pw2_len);
	memset(wsclist, 0, sizeof(float) * cwt_list_rows * cnv_pw2_len);
	for(size_t k = 0; k < cwt_list_rows; k++)
	{
        tmp = (float)k;
		memcpy(wsclist + (k * cnv_pw2_len),
			   MexH(wlen, wscale_start + (tmp * wscale_step)),
			   sizeof(float) * wlen);
	}
	wsplist = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * sp_len *\
										   cwt_list_rows);
	wp = fftwf_plan_many_dft_r2c(1, (const int*)&cnv_pw2_len, cwt_list_rows,
								 wsclist, NULL,	1, cnv_pw2_len, wsplist, NULL,
								 1, sp_len, FFTW_ESTIMATE);
	fftwf_execute(wp);

	s = (float*)fftwf_malloc(sizeof(float) * cnv_pw2_len * 2);
	memset(s, 0, sizeof(float) * cnv_pw2_len * 2);
	ssp = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * sp_len * 2);
	sp = fftwf_plan_many_dft_r2c(1, (const int*)&cnv_pw2_len, 2, s, NULL, 1,
								 cnv_pw2_len, ssp, NULL, 1, sp_len,
								 FFTW_ESTIMATE);

	xp = (float*)fftwf_malloc(sizeof(float) * cnv_pw2_len * cwt_list_rows * 2);
	xps = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * sp_len *\
									   cwt_list_rows * 2);
	iwsp = fftwf_plan_many_dft_c2r(1, (const int*)&cnv_pw2_len,
								   cwt_list_rows * 2,	xps, NULL, 1, sp_len,
								   xp, NULL, 1, cnv_pw2_len, FFTW_ESTIMATE);

	sgl_sp = fftwf_plan_dft_r2c_1d(cnv_pw2_len, s, ssp, FFTW_ESTIMATE);
	sgl_iwsp = fftwf_plan_many_dft_c2r(1, (const int*)&cnv_pw2_len,
									   cwt_list_rows, xps, NULL, 1, sp_len,
                                       xp, NULL, 1, cnv_pw2_len, FFTW_ESTIMATE);

	ch1_out_full = new float*[cwt_list_rows];
	ch2_out_full = new float*[cwt_list_rows];

	ch1_out_fit = new float*[cwt_list_rows];
	ch2_out_fit = new float*[cwt_list_rows];

	ch1_out_rdc = new float*[cwt_list_rows];
	ch2_out_rdc = new float*[cwt_list_rows];

	for(size_t k = 0; k < cwt_list_rows; k++)
	{
		ch1_out_full[k] = new float[cnv_len];
		ch2_out_full[k] = new float[cnv_len];

		ch1_out_fit[k] = new float[slen];
		ch2_out_fit[k] = new float[slen];

		ch1_out_rdc[k] = new float[cnv_red_len];
        ch2_out_rdc[k] = new float[cnv_red_len];
	}
}

TCwt::~TCwt()
{
	fftwf_destroy_plan(wp);
	fftwf_destroy_plan(sp);
	fftwf_destroy_plan(iwsp);
	fftwf_destroy_plan(sgl_sp);
	fftwf_destroy_plan(sgl_iwsp);

	fftwf_free(wsclist);
	wsclist = NULL;
	fftwf_free(wsplist);
	wsplist = NULL;
	fftwf_free(s);
	s = NULL;
	fftwf_free(ssp);
	ssp = NULL;
	fftwf_free(xp);
	xp = NULL;
	fftwf_free(xps);
	xps = NULL;

	if(wave_temp != NULL)
		delete[] wave_temp;
	wave_temp = NULL;

	for(size_t k = 0; k < cwt_list_rows; k++)
	{
		delete[] ch1_out_full[k];
		ch1_out_full[k] = NULL;
		delete[] ch2_out_full[k];
		ch2_out_full[k] = NULL;

		delete[] ch1_out_fit[k];
		ch1_out_fit[k] = NULL;
		delete[] ch2_out_fit[k];
		ch2_out_fit[k] = NULL;

		delete[] ch1_out_rdc[k];
		ch1_out_rdc[k] = NULL;
		delete[] ch2_out_rdc[k];
		ch2_out_rdc[k] = NULL;
	}
	delete[] ch1_out_full;
	ch1_out_full = NULL;
	delete[] ch2_out_full;
	ch2_out_full = NULL;
	delete[] ch1_out_fit;
	ch1_out_fit = NULL;
	delete[] ch2_out_fit;
	ch2_out_fit = NULL;
	delete[] ch1_out_rdc;
	ch1_out_rdc = NULL;
	delete[] ch2_out_rdc;
	ch2_out_rdc = NULL;
}

bool TCwt::IsPow2(size_t val)
{
	size_t k;
	bool pow_indicator = false;

	if(val >= 2)
	{
		k = val;
		while((k % 2) == 0)
		{
			k /= 2;
			if(k == 1)
			{
				pow_indicator = true;
				break;
			}
		}
    }
	return pow_indicator;
}

float *TCwt::MexH(size_t len, float a)
{
	float ampl, arg, sh;

	ampl = MEX_H_AMPL * (1.0f / a);
	sh = (float)len;
	sh /= 2.0f;

	if(wave_temp != NULL)
		delete[] wave_temp;
	wave_temp = NULL;

	wave_temp = new float[len];
	for(size_t k = 0; k < len; k++)
	{
		arg = (float)k;
		arg = (arg - sh) / a;
		wave_temp[k] = ampl * (1.0f - (arg * arg)) * exp( -(arg * arg) / 2.0f);
	}

	return wave_temp;
}

void TCwt::ExecuteBothChCwt(const float *s1, const float *s2)
{
	size_t sp2_idx, wsp_idx, xp1sp_idx, xp2sp_idx;
	float s1re, s1im, s2re, s2im, wre, wim;

	memcpy(s, s1, sizeof(float) * slen);
	memcpy(s + cnv_pw2_len, s2, sizeof(float) * slen);

	fftwf_execute(sp);

	for(size_t q = 0; q < cwt_list_rows; q++)
		for(size_t k = 0; k < sp_len; k++)
		{
			sp2_idx = k + sp_len;
			wsp_idx = k + (q * sp_len);
			xp1sp_idx = wsp_idx;
			xp2sp_idx = k + ((q + cwt_list_rows) * sp_len);

			s1re = ssp[k][0];
			s1im = ssp[k][1];

			s2re = ssp[sp2_idx][0];
			s2im = ssp[sp2_idx][1];

			wre = wsplist[wsp_idx][0];
			wim = wsplist[wsp_idx][1];

			xps[xp1sp_idx][0] = (s1re * wre - s1im * wim) / fft_norm_factor;
			xps[xp1sp_idx][1] = (s1re * wim + s1im * wre) / fft_norm_factor;

			xps[xp2sp_idx][0] = (s2re * wre - s2im * wim) / fft_norm_factor;
			xps[xp2sp_idx][1] = (s2re * wim + s2im * wre) / fft_norm_factor;
		}

	fftwf_execute(iwsp);
}

float **TCwt::ExecuteSingleChCwt(const float *sig, cwt_len_type wlt)
{
	size_t wsp_idx;
	float sre, sim, wre, wim;

	memcpy(s, sig, sizeof(float) * slen);

	fftwf_execute(sgl_sp);

	for(size_t q = 0; q < cwt_list_rows; q++)
		for(size_t k = 0; k < sp_len; k++)
		{
			wsp_idx = k + (q * sp_len);

			sre = ssp[k][0];
			sim = ssp[k][1];

			wre = wsplist[wsp_idx][0];
			wim = wsplist[wsp_idx][1];

			xps[wsp_idx][0] = (sre * wre - sim * wim) / fft_norm_factor;
			xps[wsp_idx][1] = (sre * wim + sim * wre) / fft_norm_factor;
		}

	fftwf_execute(sgl_iwsp);

	return GetCh1CwtResult(wlt);
}

float **TCwt::GetCh1CwtResult(cwt_len_type wlt)
{
	float **retval;

	switch(wlt)
	{
		case FULL:
			for(size_t k = 0; k < cwt_list_rows; k++)
				memcpy(ch1_out_full[k], xp + (k * cnv_pw2_len),
					   sizeof(float) * cnv_len);
			retval = ch1_out_full;
			break;

		case FIT:
			for(size_t k = 0; k < cwt_list_rows; k++)
				memcpy(ch1_out_fit[k], xp + (k * cnv_pw2_len) + (wlen / 2),
					   sizeof(float) * slen);
			retval = ch1_out_fit;
			break;

		case REDUCED:
			for(size_t k = 0; k < cwt_list_rows; k++)
				memcpy(ch1_out_rdc[k], xp + (k * cnv_pw2_len) + wlen - 1,
					   sizeof(float) * cnv_red_len);
			retval = ch1_out_rdc;
			break;
	}
	return retval;
}

float **TCwt::GetCh2CwtResult(cwt_len_type wlt)
{
    float **retval;

	switch(wlt)
	{
		case FULL:
			for(size_t k = 0; k < cwt_list_rows; k++)
				memcpy(ch2_out_full[k],
					   xp + ((k + cwt_list_rows) * cnv_pw2_len),
					   sizeof(float) * cnv_len);
			retval = ch2_out_full;
			break;

		case FIT:
			for(size_t k = 0; k < cwt_list_rows; k++)
				memcpy(ch2_out_fit[k],
					   xp + ((k + cwt_list_rows) * cnv_pw2_len) + (wlen / 2),
					   sizeof(float) * slen);
			retval = ch2_out_fit;
			break;

		case REDUCED:
			for(size_t k = 0; k < cwt_list_rows; k++)
				memcpy(ch2_out_rdc[k],
					   xp + ((k + cwt_list_rows) * cnv_pw2_len) + wlen - 1,
					   sizeof(float) * cnv_red_len);
			retval = ch2_out_rdc;
			break;
	}
	return retval;
}

size_t TCwt::GetFullLen(void)
{
	return cnv_len;
}

size_t TCwt::GetFitLen(void)
{
	return slen;
}

size_t TCwt::GetReducedLen(void)
{
    return cnv_red_len;
}

size_t TCwt::GetCWTRowCnt(void)
{
    return cwt_list_rows;
}

