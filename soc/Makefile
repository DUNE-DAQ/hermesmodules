.DEFAULT_GOAL := all


CXXFLAGS = -g -O2 -Isrc -std=c++17
LFLAGS = 

# output binary for each of these (no headers)
BSRC = $(wildcard src/*.cxx)
BOBJ = $(addprefix build/,$(notdir $(BSRC:.cxx=.o)))
BDEP = $(BOBJ:.o=.d)

# component object for each src/*.cc
LSRC = $(filter-out $(BSRC),$(wildcard src/*.cc))
LOBJ = $(addprefix build/,$(notdir $(LSRC:.cc=.o)))
LDEP = $(LOBJ:.o=.d)

BINS = $(notdir $(basename $(BSRC)))


.PHONY: help
help: ## display this Help Message
	@IFS=$$'\n'; for line in `grep -h -E '^[a-zA-Z_#-]+:?.*?## .*$$' $(MAKEFILE_LIST)`; do if [ "$${line:0:2}" = "##" ]; then \
	echo $$line | awk 'BEGIN {FS = "## "}; {printf "\n\033[33m%s\033[0m\n", $$2}'; else \
	echo $$line | sed -e 's/$${KIND_CLUSTER_NAME}/${KIND_CLUSTER_NAME}/' | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'; fi; \
	done; unset IFS;

all: $(BINS)  ## Build UDP server
		@echo Finished building

install-wib: ## Install WIB startup script
		install -d "$(DESTDIR)/bin/"
		install -m 0755 hermes_udp_srv "$(DESTDIR)/bin/"
		install -m 0755 init_scripts/hermes_udp_srv_wib.sh "/etc/init.d"
		ln -s ../init.d/hermes_udp_srv_wib.sh "/etc/rc3.d/S99hermes_udp_srv_wib.sh"
		ln -s ../init.d/hermes_udp_srv_wib.sh "/etc/rc5.d/S99hermes_udp_srv_wib.sh"

install-daphne: ## Install DAPHNE startup script
		install -d "$(DESTDIR)/bin/"
		install -m 0755 hermes_udp_srv "$(DESTDIR)/bin/"
		install -m 0755 init_scripts/hermes_udp_srv_daphne.sh "/etc/init.d"
		ln -s ../init.d/hermes_udp_srv_daphne.sh "/etc/rc3.d/S99hermes_udp_srv_daphne.sh"
		ln -s ../init.d/hermes_udp_srv_daphne.sh "/etc/rc5.d/S99hermes_udp_srv_daphne.sh"

install-zcu102: ## Install ZCU102 startup script
		install -d "$(DESTDIR)/bin/"
		install -m 0755 hermes_udp_srv "$(DESTDIR)/bin/"
		install -m 0755 init_scripts/hermes_udp_srv_zcu102.sh "/etc/init.d"
		ln -s ../init.d/hermes_udp_srv_zcu.sh "/etc/rc3.d/S99hermes_udp_srv_zcu.sh"
		ln -s ../init.d/hermes_udp_srv_zcu.sh "/etc/rc5.d/S99hermes_udp_srv_zcu.sh"

clean: ## Clean products
		@rm -rf build $(PBGENS)
		@rm -rf build $(BINS)

# binaries depend on all component objects
$(BINS): %: build/%.o $(LOBJ) $(PGOBJS)
		$(CXX) $< $(LOBJ) $(LFLAGS) -o $@

$(BDEP): build/%.d: src/%.cxx $(PBGENS)
		@mkdir -p build
		@set -e; rm -f $@
		$(CXX) -M $(CXXFLAGS) -MT $(<:.cxx=.o) $< > $@
		@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@


$(LDEP): build/%.d: src/%.cc src/%.h $(PBGENS)
		@mkdir -p build
		@set -e; rm -f $@
		$(CXX) -M $(CXXFLAGS) -MT $(<:.cc=.o) $< > $@
		@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

# these won't exist the first build
-include $(LDEP) $(BDEP)

$(BOBJ): build/%.o: build/%.d $(PBGENS)
		$(CXX) $(CXXFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.cxx))) -o $@

$(LOBJ): build/%.o: build/%.d $(PBGENS)
		$(CXX) $(CXXFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.cc))) -o $@
