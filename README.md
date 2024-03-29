# tinycc_win32

Updated `Tiny C Compiler` from [Fabrice BELLARD](https://github.com/bellard) (`win32` version)

Benefits of `tcc` : simple, portable (<2MB compiler + basic includes) and pretty damn fast.<br>
Beware : while functional for basic usages, `tcc` have many limitations and bugs when pushing it to its limits.<br>
Warning : this is the `mob` branch, which is open to experimental/unsupervised changes.<br>

  - Original [tcc](https://bellard.org/tcc/)
  - Grishka [tinycc](https://repo.or.cz/tinycc.git)
  - New mirror [tinycc](https://github.com/mirror/tinycc)
  - Old mirror [tinycc](https://github.com/TinyCC/tinycc)
  - Savannah [tinycc](https://savannah.nongnu.org/projects/tinycc)
  - Releases [tinycc](http://download.savannah.nongnu.org/releases/tinycc/)
  
Base version is : `tinycc-7e90129.zip - 2020-01-22 	grischka	Rework expr_infix (mob)`<br>
Compiler is : `tcc_0.9.27 - 17-Dec-2017 08:27` (latest public release)<br>

1- Edit `config.h` and `./win32/make-tcc.bat` files if necessary<br>
2- To compile `tcc`, just run `./win32/make-tcc.bat`, it will "self" compile<br>
3- Compiled `tcc` will be located into `./win32/`<br>
4- If you want to use `tcc` as a compiler, use `./win32/tcc.exe` with the `./win32/include/` folder.

You can check the [Savannah bugtracker](https://savannah.nongnu.org/bugs/?group=tinycc) but it isn't followed by the current maintainers, hence old bugs remains.<br>
This repo won't provide bugfixes either, but mostly improve the whole distribution with header files and libraries.<br>
