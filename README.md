# rc-web
Web server originally created for rendering my collection of clips from the video game Rust, now is just a fun project to implement things into. 
Written in C because I can and because control starts with c. 

## Why
I made this after I watching a Low Level Learning video that claimed that writing an HTTP server in C was a great way to learn C. 
It was. It was also antagonizingly difficult. HTTP was the starting point, and now HTTPS has been implemented (insecurely). If exposing this to the
outside, prepare for a silly amount of web crawlers.

## Use
I have a makefile but its very jumbled. For barebones HTTPS server I would remove alccalc, hashtable, users, and external libraries.
main.c is the entry point and maintains the server's connections. Everything else happens elsewhere in a messed up head scratching kind of way

