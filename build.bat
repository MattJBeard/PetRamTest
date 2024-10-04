@echo off

del %1.o65
c:\cc65\bin\ca65 -v --cpu 65c02 %1.a65 -o %1.o65 -l %1.lst

if exist %1.o65 c:\cc65\bin\ld65 -v -vm -C ld65.cfg %1.o65 -o %1.bin -m %1.map
