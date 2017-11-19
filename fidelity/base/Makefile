
INCL = $(STAGE_LIB)/include/fidelity/$(notdir $(CURDIR))

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

TSRCS = $(wildcard test/*.cpp)
TOBJS = $(TSRCS:%.cpp=$(OBJDIR)/%.o)
COBJS = $(SRCS:%.cpp=$(BUILD)/cov/%.o) $(TSRCS:%.cpp=$(BUILD)/cov/%.o)

DEPS = $(OBJS:.o=.d) $(TOBJS:.o=.d) $(COBJS:.o=.d)

out: $(OBJS)
	mkdir -p $(INCL)
	cp -u -a $(wildcard *.hpp) $(INCL)

$(OBJDIR)/%.o: %.cpp Makefile | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)

$(BUILD)/cov/%.o: %.cpp Makefile | $(BUILD)/cov
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)

test: $(TOBJS)

$(OBJDIR)/test/%.o: test/%.cpp Makefile | $(OBJDIR)/test
	$(CXX) $(CXXFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)

$(BUILD)/cov/test/%.o: test/%.cpp Makefile | $(BUILD)/cov/test
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)

$(OBJDIR) $(OBJDIR)/test:
	mkdir -p $(OBJDIR)/test

$(BUILD)/cov $(BUILD)/cov/test:
	mkdir -p $(BUILD)/cov/test

cov: $(COBJS)

clean:
	rm -rf $(OBJDIR)

.PHONY: test

-include $(DEPS)
