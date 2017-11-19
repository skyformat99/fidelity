
DEMO = $(STAGE_TEST)/fidelity/$(notdir $(CURDIR)).demo

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

DEPS = $(OBJS:.o=.d)

demo: $(DEMO)
	
$(DEMO): $(OBJS)
	mkdir -p $(dir $(DEMO))
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) -lstdc++fs -lfidelity

$(OBJDIR)/%.o: %.cpp Makefile | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)
	
$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

-include $(DEPS)