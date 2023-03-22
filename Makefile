CXX = g++
CXXFLAGS = -Ofast

negamax.out: negamax.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

.PHONY: run
run: negamax.out
	./negamax.out

.PHONY: clean
clean:
	rm -f negamax.out
