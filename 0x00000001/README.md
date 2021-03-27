# Bug
## file
code/mkfs.c line 91. 

A 64GB disk should have about 2 million inodes, but it told me about only 50000. 
I use Python Shell to calculated it, it also told me about 2 million. But it was
50000 when I use C. 

I used GDB, but it's not work. GDB is not an assembly debugger, it just told me 
errors like Segmentation-Fault or others. This isn't an error, only human thinks 
it does. 


# Answer

Then, I read Assembly code. At mkfs.s line 223, I think I found that error. `SAL` 
instruction takes a signed number, so it will overflow when it's too big. But other
numbers are all unsigned, you know. 

Answer of instruction `cblocks * BLOCK_SIZE` is too big to `int` (32bits signed), 
it overflows. 

# Lessons

The all numbers should be signed or unsigned, else it maybe overflow. And you must 
know some assembly if you want use C better :-). 

