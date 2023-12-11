#include "Blend.h"
#include <dshow.h>


int gnTimer = 0, g_nSwapSteps = 0, g_nSwapState = SWAP_NOTHING;

void SwapStreams()
{
	STRM_PARAM strSwap = { 0 };

	strSwap = strParam[PRIMARY_STREAM];
}