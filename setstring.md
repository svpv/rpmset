## On the maximum length of Golomb-Rice code

**Lemma.** The maximum bitlength of a set of `n` numbers in the range
`(`v<sub>0</sub>`,`v<sub>max</sub>`]` encoded with Golomb-Rice code using
`m`-bit remainders is `C + Q`, where
* `C = n * (m + 1)` is the fixed-size part which accounts for `m`-bit
  remainders and unary stop-bits;
* `Q = (`v<sub>max</sub> `-` v<sub>0</sub> `- n) >> m` is the maximum number
  of variable-length q-bits.

Staring with the initial value v<sub>0</sub>, deltas must cover the range up to
and including v<sub>max</sub>.  The worst case (resulting in maximum bitlength)
is when all `m`-bit remainders are small, and we have to expend extra q-bits to
cover the range.  But since each delta gets implicitly incremented with `+1`
in the decoder (because the set of numbers is unique), the `m`-bit remainders cover
the range of at least `n`.  Therefore, the q-bits must be able to cover the
range of v<sub>max</sub> `-` v<sub>0</sub> `- n`.  Each q-bit covers 2<sup>m</sup>.
**&#8718;**

For a set of `bpp`-bit numbers, we have v<sub>0</sub>`= -1` and
v<sub>max</sub>`=`2<sup>bpp</sup>`- 1`.  Therefore, the maximum bitlength is
`C + (`2<sup>bpp</sup>` - n) >> m`.  If the last array element `v[n-1]` is
known in advance, we get a slightly tighter bound: `C + (v[n-1] + 1 - n) >> m`.

(We have v<sub>0</sub>`= -1`, not v<sub>0</sub>`= 0`, because each delta
gets implicitly incremented with `+1`; if `dv[0]` is `0`, the first iteration
`v[i] = (v0 += dv[i] + 1)` gets us `v[0] = 0`, as expected.)

## The loop control of a block decoder

Suppose we have the `unpack` primitive which consumes `kc` characters and
produces a block of `kn` `m`-bit numbers.  We want the decoder to process
the input in blocks: to unpack `kn` `m`-bit remainders and complement them with
q-bits from the bitstream.  (The bitstream in consumed from the input in
smaller chunks, which are interleaved with the blocks.)  Near the end of the
input, the decoder should switch to a slower code path and consume `m`-bit
quantities from the bitstream, one by one.

Let len<sub>1</sub> be the number of remaining characters in the input.
The problem is that the condition len<sub>1</sub>` >= kc` is not sufficient
to conclude whether we should process another block: after unpacking the block,
we may find out that we've run out of q-bits.  We may try to redistribute the
`m` bits from the last decoded delta to the bitstream, but there is no hard
limit on how many deltas we may need to redistribute.  In the worst case,
we may need to reassign bits from more than one block.

Our soultion is based on the above Lemma.  We must not process a block if
there is a possibility that only `kn - 1` elements are left to decode.
Let len<sub>0</sub> be the maximimum length of Golomb-Rice code for `kn - 1`
numbers.  (Note that len<sub>0</sub> depends on the last decoded value
v<sub>0</sub>, and should be re-evaluated on each iteration.)  Therefore the
decoder should process the next block iff len<sub>1</sub>` > `len<sub>0</sub>.

The encoder works in lockstep with the decoder.  It first calculates the output
length (using a simplified procedure which does not assume blocking).  It then
writes the output, replicating the decisions that will be made in the decoder
based on the remaining output length.
