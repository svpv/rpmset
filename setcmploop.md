# Comparing two sets

A set is represented by an array of increasing integers.  Given two sets `P`
and `R`, we usually want to check that `P >= R`, or in set notation, `R ⊆ P`
(Requires is a subset of Provides).  For completeness, and for integration with
the package manager, we also have to recognize other outcomes, such as `P < R`.
We therefore design a three-way comparison routine which returns `1` when `P > R`,
`-1` when `P < R`, and `0` when `P == R`.

To perform comparison, we utilize a [merge algorithm]: at each step, two
current elements, that of `P` and that or `R`, are compared.  If `P < R`,
`P` gets advanced; if `P > R`, `R` gets advanced (in other words, the smaller
one catches up, so that the elements are matched in order); otherwise, when
`P == R`, both pointers are advanced.

[merge algorithm]: https://en.wikipedia.org/wiki/Merge_algorithm#Merging_two_lists

## A naive version

Two sets are equal iff `P ⊆ R` and `P ⊇ R`.  At the beginning of the loop,
we assume that the sets may be equal, and set the `le` and `ge` flags to `1`.
The flags are to be cleared as the comparison progresses and mismatches
are detected.

```c
int setcmploop(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn)
{
    bool le = 1, ge = 1;
    const uint32_t *Pend = Pv + Pn;
    const uint32_t *Rend = Rv + Rn;
    while (Pv < Pend && Rv < Rend)
	if (*Pv < *Rv)
	    le = 0, Pv++;
	else if (*Pv > *Rv)
	    ge = 0, Rv++;
	else
	    Pv++, Rv++;
    if (Pv < Pend) le = 0;
    if (Rv < Rend) ge = 0;
    if (le && ge) return 0;
    if (ge) return 1;
    if (le) return -1;
    return -2;
}
```
Consider the case of `*Pv < *Rv`.  If the condition holds, it means that `P`
has an element which is not in `R`.  Therefore `P` is not "less than or equal
to" `R`, so the `le` flag is cleared.  After the loop is complete, if there are
any remaining elements in `P`, `le` is cleared as well.

In the end, if at most one flag has been cleared, we've got a clean result:
`P == R`, `P > R`, or `P < R`.  If however both flags have been cleared, this
means that `P` has an element which is not in `R` while `R` has an element
which is not in `P`, so neither is a subset of each other.  In this case,
we return `-2` to indicate that the sets are just "not equal".

Note that the condition in the `while` loop always tests two boundaries.
This is redundant when only one pointer gets advanced.  We can help the
compiler by explicitly spelling which conditions must be checked in each case.
(In the following, we also assume that both sets are non-empty.)
```c
    while (1)
	if (*Pv < *Rv) {
	    le = 0, Pv++;
	    if (Pv == Pend) break;
	}
	else if (*Pv > *Rv) {
	    ge = 0, Rv++;
	    if (Rv == Rend) break;
	}
	else {
	    Pv++, Rv++;
	    if (Pv == Pend) break;
	    if (Rv == Rend) break;
	}
```
It now takes two comparisons to execute the `if` branch: `*Pv < *Rv` and
`Pv == Pend`.

## Breaking the symmetry

While the cases of `*Pv < *Rv` and `*Pv > *Rv` are logically symmetric,
the former is executed much more often.  This is because the Requires set is
typically sparse, i.e. contains only a small subset of Provides.  We can take
advantage of this asymmetry by introducing a tight inner loop which skips
`Pv` elements:

```c
    uint32_t Rval = *Rv;
    assert(Pv[Pn] == UINT32_MAX);
    while (1) {
	if (*Pv < Rval) {
	    le = 0;
	    do
		Pv++;
	    while (*Pv < Rval);
	    if (Pv == Pend) break;
	}
	if (*Pv > Rval) {
	    ge = 0, Rv++;
	    if (Rv == Rend) break;
	    Rval = *Rv;
	}
	else {
	    Pv++, Rv++;
	    if (Pv == Pend) break;
	    if (Rv == Rend) break;
	    Rval = *Rv;
	}
    }
```
We have introduced a separate register for `Rval = *Rv`, because it changes at
a rate lower than `Pv`.  Once the case of `*Pv < Rval` is entered and `le` is
cleared, a few more `Pv` elements are skipped quickly, until we encounter
`*Pv >= Rval`, or until `Pv` is at `Pend`.  Both of these conditions can be
tested with a single comparison, `*Pv < Rval`, if we install a sentinel at the
end of `Pv` array: `Pv[Pn] = UINT32_MAX`.

## Unrolling the loop

The inner loop can further be unrolled, as explained in *Knuth, Vol.3, p.398*.
In our experience, unrolling by a factor of 3 or 4 works best.

```c

	if (*Pv < Rval) {
	    le = 0;
	    while (1) {
		if (Pv[1] >= Rval) { Pv += 1; break; }
		if (Pv[2] >= Rval) { Pv += 2; break; }
		if (Pv[3] >= Rval) { Pv += 3; break; }
		Pv += 3;
	    }
	    if (Pv == Pend) break;
	}
```
One reason why unrolling helps is that the load of `Pv[2]` can be issued
speculatively, in parallel with the test for `Pv[1] >= Rval`.  This isn't
possible without unrolling, because the load from the next `*Pv` has to wait for
the increment to complete.

## Stepping over

In practice, `P` sets are about 20-30 times bigger than `R` sets (on average).
To skip `Pv` elements more aggressively, the inner loop can step over another
element, that is, test only every second element.

```c
	if (*Pv < Rval) {
	    le = 0;
	    do
		Pv += 2;
	    while (*Pv < Rval);
	    Pv = (Pv[-1] >= Rval) ? Pv - 1 : Pv;
	    if (Pv == Pend) break;
	}
```
For the loop to work, two `UINT32_MAX` sentinels must be installed at the end
of `Pv` array.  When the loop completes, we may need to step one element back.
Here is the unrolled version:

```c
	if (*Pv < Rval) {
	    le = 0;
	    while (1) {
		if (Pv[2] >= Rval) { Pv += 2; break; }
		if (Pv[4] >= Rval) { Pv += 4; break; }
		if (Pv[6] >= Rval) { Pv += 6; break; }
		Pv += 6;
	    }
	    Pv = (Pv[-1] >= Rval) ? Pv - 1 : Pv;
	    if (Pv == Pend) break;
	}
```
The performance of this approach depends critically on the ability of the compiler
to handle stepping back with a `cmov` instruction.  Unfortunately, this is not
the case with the above code, so we try to cajole the compiler into emitting `cmov`
by morphing the "step back" into a "step forth":

```c
	if (*Pv < Rval) {
	    le = 0;
	    while (1) {
		if (Pv[2] >= Rval) { Pv += 1; break; }
		if (Pv[4] >= Rval) { Pv += 3; break; }
		if (Pv[6] >= Rval) { Pv += 5; break; }
		Pv += 6;
	    }
	    Pv = (*Pv < Rval) ? Pv + 1 : Pv;
	    if (Pv == Pend) break;
	}
```

## Bigger steps

Generally we can can use the step size of 2<sup>n</sup> and bisect to the right
position in `Pv` with n `cmov` instructions.  The approach is similar to Binary
merging in *Knuth, Vol.3, p.203*.  We have implemented a variant with step=4
in assembly.  Its merit depends on how cheap the underlying `cmov` instruction
is.  On Haswell, `cmov` has 2-cycle latency, and step=4 beats step=2 when
`Pn/Rn > 32`.  On Ryzen and Skylake, `cmov` has 1-cycle latency, so step=4
rivals step=2 at smaller `Pn/Rn` ratios.

## Benchmarks

For benchmarking we use a real-world data set obtained from `apt-cache unmet`.
There are 83899 `setcmploop` calls, with 1519±2539 elements on average in the
`P` set and 51±173 elements in the `R` set.  We report the mean time per call,
according to `rdtsc`.  The compiler is gcc 9.3 for `x86_64`, the CPU is Haswell.

| setcmploop variant                          | cycles per call |
| ------------------------------------------- | --------------- |
| naive0                                      | 4100            |
| naive1                                      | 3800            |
| step=1                                      | 3030            |
| step=1 unrolled                             | 2660            |
| step=2                                      | 2900            |
| step=2 unrolled                             | 2510            |
| [setcmploop.c](setcmploop.c)                | 2440            |
| [setcmploop-x86\_64.S](setcmploop-x86_64.S) | 2310            |
