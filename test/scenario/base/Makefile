
TSRCS = $(wildcard *.cpp)
TOBJS = $(TSRCS:%.cpp=$(OBJDIR)/%.o)

DEPS = $(OBJS:.o=.d)

test: $(TOBJS)

$(OBJDIR)/%.o: %.cpp Makefile | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)
	
$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

-include $(DEPS)