Reasons for this Fork:




1. Bring back the good old wideview with following loving features
- Sort the Feeds by name, not by date.
- Bring back the date-column
- Hide the teaser under the Feeds
- Feednames only needs one row
- Smaller Favicons
- Bring back the state-column
2. It is actually on my Gentoo-System not possible anymore to build the last 1.10.19 Version and i was really unhappy 
with the Versions in the gentoo repositorys. So i made small changes and deleted some code snippets. It is at this time pretty
nasty code, but it is fully working. When i have time, i clean up the unused "playing" code snippets.


![screenshot](https://i.imgur.com/sCT5E9b.png)



For more build instructions take a look to lwindolf's git repository.

But it is the same process like the normal liferea building process:

Build it with:


./autogen.sh
make
make install

