#
#  $Id$
# 
PRG	= fakeService
CFLAGS	= -O2 -Wall 

HEADERS	= fake.h
SOURCES	= fake.c config.c
OBJECTS = fake.o config.o

$(PRG): $(OBJECTS)
	@printf "Linking   $@:"
	@$(CC) $(LDFLAGS) $(CFLAGS) $(OBJECTS) -o $@
	@printf "\tdone.\n"

clean:
	@rm -f  $(OBJECTS) $(PRG) *~

.c.o:
	@printf "Compiling $<:"
	@$(CC) $(CFLAGS) -Os -c $< -o $@
	@printf "\tdone.\n"
