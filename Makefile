.PHONY: playdate raylib clean

playdate:
	$(MAKE) -f Makefile_playdate all

raylib:
	$(MAKE) -f Makefile_raylib all

all:
	$(MAKE) -f Makefile_playdate pdx-sim
	$(MAKE) -f Makefile_playdate pdx-dev
	$(MAKE) -f Makefile_raylib all


clean:
	$(MAKE) -f Makefile_playdate clean
	$(MAKE) -f Makefile_raylib clean