## On the maximum length of Golomb-Rice code

**Lemma.** The maximum bitlength of a set of `n` numbers in the range
`(`v<sub>0</sub>`,`v<sub>max</sub>`]` encoded with Golomb-Rice code using
`m`-bit deltas is `C + Q`, where
* `C = n * (m + 1)` is the fixed-size part which accounts for `m`-bit
  deltas and unary stop-bits;
* `Q = (`v<sub>max</sub> `-` v<sub>0</sub> `- n) >> m` is the maximum number
  of variable-length q-bits.

Staring with the initial value v<sub>0</sub>, deltas must cover the range up to
and including v<sub>max</sub>.  The worst case (resulting in maximum bitlength)
is when all `m`-bit deltas are small, and we have to expend extra q-bits to
cover the range.  But since each delta gets implicitly incremented with `+1`
in the decoder (because the set of numbers is unique), the `m`-bit deltas cover
the range of at least `n`.  Therefore, the q-bits must be able to cover the
range of v<sub>max</sub> `-` v<sub>0</sub> `- n`.  Each q-bit covers 2<sup>m</sup>.
**&#8718;**

For a set of `bpp`-bit numbers, we have v<sub>0</sub>`= -1` and
v<sub>max</sub>`=`2<sup>bpp</sup>`- 1`.  Therefore, the maximum bitlength is
`C + (`2<sup>bpp</sup>` - n) >> m`.  If the last array element `v[n-1]` is
known in advance, we get a slightly tighter bound: `C + (v[n-1] + 1 - n) >> m`.

(We have v<sub>0</sub>`= -1`, not v<sub>0</sub>`= 0`, because each delta
gets implicitly incremented with `+1`; if `dv[0] is `0`, the first iteration
`v[i] = (v0 += dv[i] + 1)` gets us `v[0] = 0`, as expected.)
