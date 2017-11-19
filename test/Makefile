
TESTDIRS = scenario/base
DEMODIRS = demo/loop

TEST  = $(STAGE_TEST)/fidelity/fidelity.scenario
TOBJS = $(wildcard $(TESTDIRS:%=%/$(OBJDIR)/*.o))

scenario: $(TEST)

$(TEST): $(TESTDIRS:%=%.test)
	mkdir -p $(dir $(TEST))
	$(CXX) -o $@ $(TOBJS) $(LDFLAGS) $(LDGTEST) -lfidelity
	
%.test:
	$(MAKE) -C $* test

demo: $(DEMODIRS:%=%.demo)

%.demo:
	$(MAKE) -C $* demo

CLEANDIRS = $(TESTDIRS) $(DEMODIRS)

clean: $(CLEANDIRS:%=%.clean)

%.clean:
	$(MAKE) -C $* clean

.PHONY: demo
