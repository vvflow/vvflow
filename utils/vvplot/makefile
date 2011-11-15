CC = icc -O3 -mkl=parallel -openmp
names:= presplot isoline streamlines
exes:= $(patsubst %,%_exe, $(names))

all: $(exes)
isoline_exe: isoline.cpp
	icc -O0 $^ -o $@ $(VVHD) -std=c++0x

%_exe: %.cpp
	$(CC) $^ -o $@ $(VVHD) -std=c++0x

clean:
	rm -f $(exes)

install:
	cp vvplot vvparse $(exes) ~/.bin

uninstall:
	rm -f $(patsubst  %,~/.bin/%, $(exes) vvplot vvparse)

