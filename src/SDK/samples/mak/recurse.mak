
CLEAN_DIRS = $(DIRS) $(RECURSE_EXTRA_CLEAN_DIRS)

define NL


endef

all: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) -SC $(i) $(NL) )

depend: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) -SC $(i) depend $(NL) )

tests: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) -SC $(i) tests $(NL) )

clean: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i,$(CLEAN_DIRS),$(MAKESH) $(MAKE) -SC $(i) clean $(NL))
