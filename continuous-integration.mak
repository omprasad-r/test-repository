CM = ../configuration-management/trunk
include  $(CM)/buildfarm.properties.mak

RUBYLIB = .:$(PWD)/../acquia-lib/lib
RUBYENV = DEST=$(DEST) LOG_LEVEL=0

$(buildername) : run

ifeq ($(buildername),gardens-system-test)
setup run teardown:
	cd system-tests/smoke; $(RUBYENV) rake "-I$(RUBYLIB)" $@ --trace
endif

ifeq ($(buildername),gardens-smoke-test)
run : buildtest;
setup buildtest teardown:
	cd system-tests/smoke; $(RUBYENV) rake "-I$(RUBYLIB)" $@ --trace
endif

ifeq ($(buildername),gardens-jsunit-test)
setup run teardown:
	cd system-tests/jsunit; $(RUBYENV) rake "-I$(RUBYLIB)" $@ --trace
endif

ifeq ($(buildername),gardens-simpletest)
setup run teardown:
	cd system-tests/simpletest; \
          EC2_ACCOUNT=gardens-dev \
          EC2_CERT=$(HOME)/ec2/gardens-dev/cert.pem \
          EC2_PRIVATE_KEY=$(HOME)/ec2/gardens-dev/pk.pem \
          NETRC=$(HOME)/ec2/gardens-dev/netrc \
          FIELDS_STAGE=gardens-simpletest \
          $(RUBYENV) \
          rake RUBYLIB=$(RUBYLIB) "-I$(RUBYLIB)" $@ --trace
	
endif
