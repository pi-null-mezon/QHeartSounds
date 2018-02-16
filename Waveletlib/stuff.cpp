#define SIG_LEN         	5000				// Lenght of a signal to be CWTed
#define WAVE_LEN			2300				// Length of a wavelet

#define SCALES_START		5.0f				// The initial value of the scaling coefficient a (should be greater than 0)
#define SCALES_STEP			2.0f				// Its increment (should be greater than 0)
#define SCALES_STOP			253.0f				// The final value of a

/* ------------------------------------------------------------------------------ */
/* ------------- Don't forget to link fftw library to your project! ------------- */
/* ------------------------------------------------------------------------------ */

#include "cwt.h"
/* ------------------------------------------------------------------------------ */
/* ------------------------- Somewhere in your program -------------------------- */
/* ------------------------------------------------------------------------------ */
	*float sig = [<value>, <value>, <value>, ...];										// Assume we already have the signal for CWT
	
	cwt = new TCwt(SIG_LEN, WAVE_LEN, SCALES_START, SCALES_STOP, SCALES_STEP);			// Let's initialize what's necessary for CWT including matrix of MEX (mexican-hat) values
	**float cwt_result = cwt->ExecuteSingleChCwt(sig, FULL);							// That's what you need to do. Just it and nothing more. The black magic is made with a single instruction
																						// If you don't want to deal with cwt tales, just cut them off by using REDUCED instead of FULL
/* ------------------------------------------------------------------------------ */
/* ------------------------- Before exiting the program ------------------------- */
/* ------------------------------------------------------------------------------ */	
	delete cwt;
