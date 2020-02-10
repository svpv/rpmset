#include <stdint.h>
#include <stddef.h>

int setcmploop(const uint32_t *Pv, size_t Pn,
	       const uint32_t *Rv, size_t Rn)
{
    int le = 1, ge = 1;
    const uint32_t *Pend = Pv + Pn;
    const uint32_t *Rend = Rv + Rn;
    uint32_t Rval = *Rv;
    while (1) {
	if (*Pv < Rval) {
	    le = 0;
	    do {
		if (Pv[2] >= Rval) goto found2;
		if (Pv[4] >= Rval) goto found4;
		Pv += 6;
	    } while (*Pv < Rval);
	    Pv -= 1;
	    goto found;
	found2:
	    Pv += 1;
	    goto found;
	found4:
	    Pv += 3;
	found:
	    Pv = (*Pv < Rval) ? Pv + 1 : Pv;
	    if (Pv == Pend)
		break;
	}
	if (*Pv == Rval) {
	    Pv++, Rv++;
	    if (Pv == Pend)
		break;
	    if (Rv == Rend)
		break;
	    Rval = *Rv;
	}
	else {
	    ge = 0;
	    Rv++;
	    if (Rv == Rend)
		break;
	    Rval = *Rv;
	}
    }
    if (Pv < Pend)
	le = 0;
    if (Rv < Rend)
	ge = 0;
    if (le && ge)
	return 0;
    if (ge)
	return 1;
    if (le)
	return -1;
    return -2;
}
