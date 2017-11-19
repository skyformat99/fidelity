
DIRS = base

LIB  = $(STAGE_LIB)/libfidelity.a
OBJS = $(wildcard $(DIRS:%=%/$(OBJDIR)/*.o))

TEST = $(STAGE_TEST)/fidelity/fidelity.unit
TCOV = $(BUILD)/cov/fidelity.unit

TOBJS = $(wildcard $(DIRS:%=%/$(OBJDIR)/test/*.o))

out: $(LIB)

$(LIB): CXXFLAGS += -Weffc++
$(LIB): $(DIRS:%=%.out)
	mkdir -p $(dir $(LIB))
	$(AR) -rcs $@ $(OBJS)

%.out:
	$(MAKE) -C $*

test: $(TEST)

$(TEST): $(DIRS:%=%.test) $(LIB)
	mkdir -p $(dir $(TEST))
	$(CXX) -o $@ $(TOBJS) $(LDFLAGS) $(LDGTEST) -lfidelity

%.test:
	$(MAKE) -C $* test

cov: $(TCOV) 

$(TCOV): $(DIRS:%=%.cov)
	mkdir -p $(dir $(TCOV))
	$(CXX) -o $@ $(BUILD)/cov/*.o $(BUILD)/cov/test/*.o $(LDFLAGS) $(LDGTEST)

%.cov:
	$(MAKE) -C $* cov

clean: $(DIRS:%=%.clean)

%.clean:
	$(MAKE) -C $* clean
