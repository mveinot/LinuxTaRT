/* ibeat.c
 * ibeat calculation routines
 * by iain truskett
 */

#include "ibeat.h"

/* iain truskett's little ibeat calculator */
int time_ibeats() {
	time_t  curtime;
	struct	tm *clk;
	float	itime;

	time(&curtime);
	clk = gmtime(&curtime);
	itime = (((clk->tm_hour + 1) * 3600*1000) + (clk->tm_min*60*1000) +
		(clk->tm_sec*1000)) / 86400.0;

	return (((int)itime) % 1000);

/*	subbeats = 100*(itime - (int)(itime));*/

}

