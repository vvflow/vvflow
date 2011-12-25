CC = icc -O3 -openmp
names:= presplot isoline streamlines
exes:= $(patsubst %,%_exe, $(names))

all: $(exes)
isoline_exe: isoline.cpp
	icc -O0 -openmp $^ -o $@ $(VVHD) -std=c++0x

%_exe: %.cpp
	$(CC) $^ -o $@ $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f $(exes)

install:
	cp vvplot vvxtract $(exes) ~/.bin

uninstall:
	rm -f $(patsubst  %,~/.bin/%, $(exes) vvplot vvxtract)

