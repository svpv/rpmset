# Comparing two sets

A set is represented by an array of increasing integers.  Given two sets `P`
and `R`, we usually want to check that `P >= R`, or in set notation, `R ⊆ P`
(Requires is a subset of Provides).  For completeness, and for integration with
a package manager, we also have to recognize other outcomes, such as `P < R`.
We therefore design a three-way comparison routine which returns `1` when `P > R`,
`-1` when `P < R`, and `0` when `P == R`.

To perform comparison, we utilize a [merge algorithm]: at each step, two
current elements, that of `P` and that or `R`, are compared.  If `P < R`,
`P` gets advanced; if `P > R`, `R` gets advanced (in other words, the smaller
one catches up, so that the elements are traversed in order); otherwise, when
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
