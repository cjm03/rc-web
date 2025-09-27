# rc-web
Web server originally created for rendering my collection of clips from the video game Rust, now is just a fun project to implement things into. 
Written in C because I can and because control starts with c. 

## Why
I made this because I watched a Low Level Learning video that claimed that writing an HTTP server in C was a great way to learn C. 
It definitely was beneficial for learning, but it was also extremely antagonizingly difficult as a beginner. HTTP was the starting point and now I 
have reached the HTTPS implementation which was surprisingly straightforward. Probably because I don't have anything I actually want to secure. 
The hardest part was honestly the request parser.

## Use
I have a makefile but I anticipate this only working on my system. If you want it to work on yours, I can only recommend matching my system:
 - Arch Linux
 - 32GB Corsair Vengeance DDR4 3200MHz RAM 
 - Intel i5-11600KF
 - RTX 3060
 - Probably more stuff
The server could be replicated extremely easily, in all seriousness. main.c is the entry point and maintains the server's connections. 
Everything else happens elsewhere.

