# MiniDB

Minimal Data Base Memory Manager. I worked on this sophomore year on a joint project between [Oracle MDC](https://www.oracle.com/goto/mdc), [ITESO](https://www.iteso.mx/) and [CONACYT PROINNOVA](https://www.conacyt.gov.py/proinnova).

## Overview

Let's start with some context. I did an internship at [Oracle MDC](https://www.oracle.com/goto/mdc) while studying at [ITESO](https://www.iteso.mx/) during sophomore year. At this internship, someone working on his thesis used [CONACYT PROINNOVA](https://www.conacyt.gov.py/proinnova)'s help to get funding for someone to help him implement it. I was the intern tasked to help with this.

**What was the original project?** The person working on his thesis was trying to use FPGAs to accelerate database query execution, however, focusing on a column-wise storage of an in-memory database that runs blazing fast SIMD operations on the FPGAs.

**What was I tasked to do?** I was supposed to help the thesis implement such SIMD operations in the FPGAs (or at least some sort of emulation environment).

**What was worked on?** The thesis was never shared, so I decided to implement a simple in-memory database with query support with colum-wise data layout; using this "mini db" I was hoping to create a playground for testing SIMD operations to speed up query execution. Because of the time it took me to implement the mini db itself, I didn't get the chance to implement SIMD on it. I did do some experiments (not on this repo) but didn't integrate them into this query engine. Regardless, implementing this gave me a lot of things to learn.

## Scope

The scope of this project was supposed to be driven by someone working on research, but had to fallback to something that came from my own will. The project then morphed to **MiniDB**. It doesn't implement SIMD, but was meant for it. It's the playground.

## Docs

Please refer to [Oracle-Iteso Dev Report MiniDB.pdf](.\Oracle-Iteso Dev Report MiniDB.pdf) file for a document describing the project in more detail.

Please refer to the [source code's doxygen documentation](.\src\html\index.html).

## Future

There are no plans to continue working on this project for the time being. Pushing to GitHub to "record" it on my personal account.
