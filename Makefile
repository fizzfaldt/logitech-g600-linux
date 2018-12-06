CXXFLAGS=
CXXFLAGS+= -Weverything
CXXFLAGS+= -Werror
CXXFLAGS+= -std=c++11
CXXFLAGS+= -Wno-c++98-compat
CXXFLAGS+= -Wno-c++98-compat-pedantic
CXXFLAGS+= -Wno-c99-extensions

CXX=clang++

.DEFAULT_GOAL := g600_root
g600: g600.cpp
#    clang -Weverything -std=c++11 g600.cpp

g600_root: g600
	@sudo -k
	@echo Put in credentials to set as run-root
	@sudo -v
	@sudo cp $< $@
	@sudo chmod u+s $@
