---
section: 32
x-masysma-name: unraf
title: Unraf
date: 2020/01/12 22:42:10
lang: en-US
author: ["Linux-Fan, Ma_Sys.ma (Ma_Sys.ma@web.de)"]
keywords: ["programs", "c", "unraf"]
x-masysma-version: 1.0.0
x-masysma-repository: https://www.github.com/m7a/bo-unraf
x-masysma-website: https://masysma.net/32/unraf.xhtml
x-masysma-owned: 1
x-masysma-copyright: |
  Copyright (c) 2013, 2020 Ma_Sys.ma.
  For further info send an e-mail to Ma_Sys.ma@web.de.
---
German Description
==================

Unraf ist ein simples C-Programm, mit dem man League of Legends Archivdateien
entpacken kann. Zur Bedienung gibt man einfach die zu entpackende Kombination
aus `.raf`-Datei und `.raf.dat`-Datei an. Damit kann man z. B.
Character-Modelle, Sounds und Texturen entpacken.

Hinweis/Warning
===============

Dieses Programm ist von 2013. Die Chance, dass die Spieldateien immernoch im
selben Format vorliegen ist gering. Von daher kann es gut sein, dass das
Programm nicht mehr funktioniert.

This program is from 2013. Chances are the game is using different data formats
by now. It is thus very likely that this program does not work anymore.

Usage Instructions
==================

## Compile

	$ ant

## Run

	$ ./unraf <.raf> <.raf.dat> [<out-dir> [-r]]

## Install

	# cp unraf /usr/local/bin

## Further information

see Sourcecode

	$ vim unraf.c

License
=======

see LICENSE.txt

	$ cat LICENSE.txt
