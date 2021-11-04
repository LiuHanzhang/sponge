Lab 3 Writeup
=============

My name: Hanzhang Liu

My SUNet ID: [your sunetid here]

This lab took me about 9 hours to do. I [did/did not] attend the lab session.
NOTE: I have originally written this lab in 2020 fall version. 
      I've decided to refactor my code on 2021 version.
      I spent 9 hours on 2020 version.

I worked with or talked about this assignment with: [please list other sunetids]

## Program Structure and Design of the TCPSender:
Thank YOU!!! This lab document is unprecedentely organized, datailed and clear. Sec 3.2 specifies what each function is responsible for, and sec 3.1 additionally defines each function's behavior in face of timer event. The program structure is basically the direction implementation of these guidelines.

In this lab, I drew a sketch of each function in the form of pseudo-code, and designed Timer Class in advance. Bellow are my sketches.

![CS144-6.jpg](https://i.loli.net/2021/09/15/E61lLcTwP7u4CY2.jpg)

![CS144-7.jpg](https://i.loli.net/2021/09/15/5NIyHnrqihFTuXL.jpg)

![CS144-8.jpg](https://i.loli.net/2021/09/15/Hzvwy28eoApFGrq.jpg)

![CS144-9.jpg](https://i.loli.net/2021/09/15/ZPC4utAhVG3lkYn.jpg)

![CS144-10.jpg](https://i.loli.net/2021/09/15/UXdLj5FTaOtcG3v.jpg)

**Window Size**

It's worth mentioning the management of window size. Note that Sender and receiver may not have the same ackno. Receiver's ackno >= sender's ackno, because it's possible that sender's ackno + next segment length > receiver's ackno, and thus the next whole segment cannot be acked. But receiver's window size reported by ack sements is based on receiver's ackno. So it's important to keep receiver's window size and ackno, which are extracted from ack segments. The expression that evaluates window size is 

```c++
while (_receiver_ackno + MAX(1, _window_size) > _next_seqno)
```

Note that we treat 0 window size as 1, but don't increment #consecutive transmissions and double RTO when timer goes off.

## Implementation Challenges:

## Remaining Bugs:
I don't think there is any in this lab! But I'm not so optimistic about previous labs. Anyway, lab4, bring it on!

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
