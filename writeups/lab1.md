Lab 1 Writeup
=============

My name: Hanzhang Liu

My SUNet ID: [your sunetid here]

This lab took me about 9.5 hours to do. I [did/did not] attend the lab session.
NOTE: I have originally written this lab in 2020 fall version. 
      I've decided to refactor my code on 2021 version.
      I spent 9.5 hours on 2020 version.
## Program Structure and Design of the StreamReassembler:

The essense of this lab is how to manage unordered substrings. There are two states that needs to be stroed in auxiliary memory: sequence number of the first byte, and the substring itself. We expect the data structure to:

1. Sort the substring by its sequence no.

2. Computationally efficiently to insert and delete elements.

3. The public interface is shown [here](../libsponge/stream_reassembler.hh).

So, the eligible data structures are:

1. **map** The map in STL is an ordered structure based on Red-Black Tree. With the key value set to seq no, we can easily sort the substrings. However, maintaing a Red-Black Tree introduces large computional overhead, as suggested in this [post](https://www.peileiscott.top/cs144-lab1/).

2. **list** The list in STL is a bidirectional linked list. So it's efficient to maintain. The list element is a pair of seq no and substring. Simple and basic, we can develop public interface on it according to our special needs.

## Implementation Challenges:

1. How to merge unordered substring when new one is inserted? This is a interval merging problem, and I was motivated by this [leetcode problem](https://leetcode-cn.com/problems/merge-intervals/solution/he-bing-qu-jian-by-leetcode-solution/).

2. Here is my illustration of my merging algorithm.

![Merging Algorithm Illustration](https://i.loli.net/2021/09/08/BVpcvgU69P1TGuS.jpg)

3. EOF Handling. This is very tricky! Thankfully, the [test case](../tests/fsm_stream_reassembler_single.cc) tells me what the program should do on these tricky cases.

4. Capacity. The handling logic is
```c++
    if(new_substring_right - _seq_no + _output.buffer_size() > _capacity) 
    { // If new_substring_right overflows, truncate it
        len -= _output.buffer_size() + new_substring_right - _seq_no - _capacity;
        new_substring_right = index + len;
        trunc = true;
    }
```

## Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
