Lab 2 Writeup
=============

My name: Hanzhang Liu

My SUNet ID: [your sunetid here]

This lab took me about 10 hours to do. I [did/did not] attend the lab session.
NOTE: I have originally written this lab in 2020 fall version. 
      I've decided to refactor my code on 2021 version.
      I spent 10 hours on 2020 version.

I worked with or talked about this assignment with: [please list other sunetids]

## Program Structure and Design of the TCPReceiver and wrap/unwrap routines:

## Implementation Challenges:
This lab should be very simple, yet it still took me 10 hours!!! The wrap/unwrap routines shouldn't take so long, and the blame is totally on me: I should work on another way when I was stuck in one solution for such a long time. However, I blame half of the extra work on TCPReceiver on the Lab document: It failed to specify TCPReceiver's behavior under special circumstances, like:

- SYN, FIN, Payload in one TCP segment? What the hell??

- What do I do after receiving FIN? According to the document, I should just end the ByteStream. Don't I have to reset the ISN? Do I still receive new TCP segments and push them into Reassembler? (This sounds fine, because the Reassembler has already closed the ByteStream, and should cause no problem.)

## Remaining Bugs:
I literally don't know! I'm soooooo afraid of Lab4 right now!

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
