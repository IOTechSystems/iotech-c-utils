SUBDIRS = container container_dl data mycomponent scheduler

.PHONY : all
all :
	@for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir; \
	done

.PHONY : clean
clean :
	@for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done