
INCL = $(STAGE_LIB)/include/extern/$(notdir $(CURDIR))

LIB  = $(STAGE_LIB)/lib$(notdir $(CURDIR)).a
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

DEPS = $(OBJS:.o=.d)

out: $(LIB)
	mkdir -p $(INCL)
	cp $(wildcard *.hpp) $(INCL)

$(LIB): $(OBJS)
	mkdir -p $(dir $(LIB))
	$(AR) -rcs $@ $(OBJS)

$(OBJDIR)/%.o: %.cpp Makefile | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(CURDIR)/$< -MMD -MT '$@' -MF $(@:.o=.d)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

-include $(DEPS)
