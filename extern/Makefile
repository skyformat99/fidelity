
DIRS = contract gtest-tap

out: $(DIRS:%=%.out)

%.out:
	$(MAKE) -C $*

clean: $(DIRS:%=%.clean)

%.clean:
	$(MAKE) -C $* clean
