Lab 4 Writeup
=============

My name: Hanzhang Liu

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

Gosh, I don't know how many hours I spent on lab4. It took me four days to start all over again from lab0. And the outcome is quite satisfying. My logic for lab2 is more clear, and my code looks more beautiful. For lab4, I passed all tests for the first time!

```shell
CPU-limited throughput                : 2.48 Gbit/s
CPU-limited throughput with reordering: 1.51 Gbit/s
```

## Program Structure and Design of the TCPConnection:
Below is the illustration of 4 prerequisites for clean shutdown. Note that only after a segment received can these prereqs be fullfilled.

![prerequisites](https://i.loli.net/2021/11/04/CRmpZPBolxvkYyq.jpg)

When the lab document talks about "abort connection", "reset", "clean close", it just means active() == false

## Implementation Challenges:


## Remaining Bugs:


- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
