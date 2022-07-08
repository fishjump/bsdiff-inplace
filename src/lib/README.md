# HOW BSDIFF WORKS

## Before we start

You can read the original paper here: https://www.daemonology.net/papers/bsdiff.pdf (PDF). However, as a typical academic work, the code is really hard to understand. What's worse is that, since it is only a 3-page paper, we have to get implementation details from the source code.

Though I don't like the source code and the underdetailed paper, I still recommend you to read it. It can give you a basic concept of how bsdiff works.

## Principle

A little change of source code, e.g., add/delete/change some lines of code, will make a big change of the compiled binary. The main cause of this situation is a binary will record a lot of address(jmp and call asm instructions). If we simply use the traditional copy-and-insert algorithm, the efficiency is bad.

A more advanced algorithm is analyzing the binary structure. For instance, disassemble both the old and new binaries, then construct the patch. However, this kind of algorithm is not easy to implement and platform-dependent.

BSDiff is a platform-independent algorithm. It distinguishes a old and new binaries into two parts, one is called "approximate matach region" and the rest part is "extra region". Let's see a quick example:

```
+--------+--------------+
|   old  | hello_world  |
+--------+--------------+
|   new  | hello_bsdiff |
+--------+--------------+

Diff str: 0 0 0 0 0 0 -21 4 -14 -3 2
Extra str: 'f'
```
The "hello_" generates the 6 '0's because they're the same in the old and new. Besides, 'b'- w' in ascii is -21, so we get -21 at the 7th byte(at index 6). The extra str just follows its literal meaning, so we get the 'f'. If the new is smaller than the old, extra str will not be generated, the length of diff str is min(len(old), len(new)), and the rest of old will be skipped. As you can see, this diff str is very separated, and highly compressible.

## Steps

The backward/forward and suffix/prefix are confusing and ambiguous. Let me clarify it based on both the paper and the code:

```
+-----+----------------------+-------------+-----------------------+
| old | <- forward extension |             | backward extension -> |
+-----+----------------+-----+ exact match +------+----------------+
| new |   suffix of    |     |             |      |   prefix of    |
|     |     fw-ext     |     |             |      |     bw-ext     |
+-----+----------------+-----+-------------+------+----------------+
```

According to the original paper, at the begining of page 2 which is also confusing, "we only record regions which contain at least 8 bytes not matching the forward-extension of the previous match", this refers nothing to the diff or extra str, we cannot say this 8-byte difference will definitely be a diff str or a extra str now. If you already know these concepts, this sentence may mislead you. This sentence only tells you that we don't have to generate a pair of diff and extra str for each match, i.e., if we find a match with minor difference, we can immediately generate a diff str but we don't have to do that, we can hold it until we find a remarkable difference or meet the EOF, then we can generate a very looooong diff str with extra str. Please remember the diff str is highly compressible, so this process makes sense.

Moreover, the implementation didn't follow the original paper exactly. The 3rd paragraph in Section 2 said that "we generate a pairwise disjoint set of "approximate matches" by extending the matches in each direction". However, in the code, we don't actually find an exact match, then extend it. Instead, we find a exact match, the length of which is len, and we compare new[cursor...cursor+len] with old[cursor...cursor+len], i.e., we find a exact match old[10...10+len] == new[0...0+len], then we compare old[0...len] with new[0...len]. It's ok that this comparison has overlap with the exact match region because the exact match region contributes 0 difference. We keep doing this until we find a region with over 8-byte difference. Now, we can generate the diff str and extra str from it, old[old_pos, pos] and new[old_cursor, cursor], which is almost the "approximate match region". The pos is the beginning of the last finded exact match region, and the cursor is the cursor when we find the last exact match region.

Once we get the above region, we can split it like the below shows:

```
| 0 ...  |                 | ... len |
|--------|-----------------|--------:|
| fw-ext | extra / overlap |  bw-ext |
```
The fw-ext and bw-ext are the forward/backward extension of this region. According to the paper, they are subjected to the requirement that every suffix of the forward-extension (and every prefix of the backwards extension) matches in at least 50% of its bytes, for example, we are going to find a lenf to let new[0...lenf] matches at least 50% of old[0...lenf], and this percentage should be as large as possible. We also do the similar thing for old[len-lenb...len] and new[len-lenb...len] to find lenb(len_backward_ext).

If an overlap occurs, we need to adjust the lenf and lenb, the process of which will be shown in code. If there's no overlap, the rest part(the part whitout [0...lenf] and [len-lenb...len]) is an extra str.

## A more patch device friendly algorithm