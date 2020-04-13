ifeq ($(OS),Windows_NT)
	PROJECTNAME=GetHome
	PROJECTEXT=.exe
	LIBS=-luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -lstdc++fs
	LFLAGS=-s -static
	ODIR=obj/windows
else
	PROJECTNAME=GetHome
	PROJECTEXT=
	LIBS=-lm -lX11 -lGL -lpthread -lpng -lstdc++fs
	LFLAGS=-s -static-libstdc++
	ODIR=obj/linux-x86_64
endif

IDIR=include
SDIR=src

CC=g++

CFLAGS=-I./$(IDIR) -std=c++17

_OBJ = main.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

bin/$(PROJECTNAME)$(PROJECTEXT): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(LFLAGS)

.PHONY: clean

clean:
ifeq ($(OS),Windows_NT)
	del obj\windows\*.o
else
	rm -f obj/linux-x86_64/*.o
endif
	
ifneq ($(OS),Windows_NT)

.PHONY: package-clean
package-clean:
	rm -rf dist

.PHONY: package
package:
	# create assets directory for linux
	mkdir -p dist/linux/$(PROJECTNAME)/assets
	
	# create assets directory for windows
	mkdir -p dist/windows/$(PROJECTNAME)/assets

	# copy dev assets to linux assets directory
	cp assets/olcBTB_character.png dist/linux/$(PROJECTNAME)/assets/
	cp assets/olcBTB_credits.png dist/linux/$(PROJECTNAME)/assets/
	cp assets/olcBTB_splash.png dist/linux/$(PROJECTNAME)/assets/
	cp assets/olcBTB_tileset1.png dist/linux/$(PROJECTNAME)/assets/
	cp assets/outdoors.json dist/linux/$(PROJECTNAME)/assets/

	# copy linux assets to windows assets directory
	cp dist/linux/$(PROJECTNAME)/assets/* dist/windows/$(PROJECTNAME)/assets/

	# copy linux binary
	cp bin/$(PROJECTNAME) dist/linux/$(PROJECTNAME)/
	
	# copy windows binary
	cp bin/$(PROJECTNAME)$(PROJECTEXT) dist/windows/$(PROJECTNAME)/

	# ZIP linux
	cd dist/linux; zip -r "../$(PROJECTNAME) (Linux)" .
	
	# ZIP windows
	cd dist/windows; zip -r "../$(PROJECTNAME) (Windows)" .
endif
