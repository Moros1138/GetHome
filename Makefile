ifeq ($(OS),Windows_NT)
	PROJECTNAME=GetHome.exe
	LIBS=-luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -lstdc++fs
	LFLAGS=-s -static
	ODIR=obj/windows
else
	PROJECTNAME=GetHome
	LIBS=-lm -lX11 -lGL -lpthread -lpng -lstdc++fs
	LFLAGS=-s
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

bin/$(PROJECTNAME): $(OBJ)
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
	mkdir -p dist/linux/GetHome/assets
	
	# create assets directory for windows
	mkdir -p dist/windows/GetHome/assets

	# copy dev assets to linux assets directory
	cp assets/olcBTB_character.png dist/linux/GetHome/assets/
	cp assets/olcBTB_credits.png dist/linux/GetHome/assets/
	cp assets/olcBTB_splash.png dist/linux/GetHome/assets/
	cp assets/olcBTB_tileset1.png dist/linux/GetHome/assets/
	cp assets/outdoors.json dist/linux/GetHome/assets/

	# copy linux assets to windows assets directory
	cp dist/linux/GetHome/assets/* dist/windows/GetHome/assets/

	# copy linux binary
	cp bin/GetHome dist/linux/GetHome/
	
	# copy windows binary
	cp bin/GetHome.exe dist/windows/GetHome/

	# ZIP linux
	cd dist/linux; zip -r "../GetHome (Linux)" .
	
	# ZIP windows
	cd dist/windows; zip -r "../GetHome (Windows)" .
endif
