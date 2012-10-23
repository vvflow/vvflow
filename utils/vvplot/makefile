CC = icc -O3 -openmp
names:= presplot isoline streamlines heatplot vorticity vvdistrib
scripts:= vvplot vvjoin
exes:= $(patsubst %,%_exe, $(names))

all: $(exes)

%_exe: %.cpp
	$(CC) $^ -o $@ $(VVHD) -std=c++0x -mkl=parallel

clean:
	rm -f $(exes)

install:
	cp $(scripts) $(exes) ~/.bin

uninstall:
	rm -f $(patsubst  %,~/.bin/%, $(exes) $(scripts))

