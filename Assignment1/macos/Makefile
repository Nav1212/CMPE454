LDFLAGS = -L. -lglfw -ldl # -lpthread
CXXFLAGS = -g -std=c++11 --stdlib=libc++ -Wall -Wno-unused-variable -Wno-write-strings -Wno-parentheses -Wno-self-assign -Wno-c++11-extensions -DMACOS #-pthread

vpath %.cpp ../src
vpath %.c   ../src/glad/src

OBJS = main.o world.o object.o ship.o asteroid.o shell.o gpuProgram.o linalg.o strokefont.o fg_stroke.o glad.o

EXEC = asteroids

all:    $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS) 

glad.o: ../src/glad/src/glad.c

clean:
	rm -f  *~ $(EXEC) $(OBJS) Makefile.bak

depend:	
	makedepend -Y ../src/*.h ../src/*.cpp 2> /dev/null
	@echo ""
	@echo "Now edit the Makefile to remove the ../src/ prefix of all of the dependences for the .o files"
	@echo ""

# DO NOT DELETE

asteroid.o: ../src/asteroid.h ../src/object.h ../src/segment.h
asteroid.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
asteroid.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
asteroid.o: ../src/gpuProgram.h ../src/seq.h
asteroid.o: ../src/headers.h ../src/glad/include/glad/glad.h
asteroid.o: ../src/headers.h ../src/glad/include/glad/glad.h
asteroid.o: ../src/object.h ../src/segment.h ../src/gpuProgram.h
asteroid.o: ../src/seq.h
fg_stroke.o: ../src/fg_stroke.h ../src/headers.h
fg_stroke.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
fg_stroke.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
fg_stroke.o: ../src/glad/include/glad/glad.h
fg_stroke.o: ../src/headers.h ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/gpuProgram.h ../src/headers.h
gpuProgram.o: ../src/headers.h ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/seq.h
gpuProgram.o: ../src/seq.h
headers.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
headers.o: ../src/glad/include/glad/glad.h
linalg.o: ../src/linalg.h
main.o: ../src/asteroid.h ../src/main.h ../src/strokefont.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
main.o: ../src/gpuProgram.h ../src/seq.h ../src/world.h ../src/ship.h
main.o: ../src/headers.h ../src/glad/include/glad/glad.h
main.o: ../src/object.h ../src/segment.h ../src/shell.h
object.o: ../src/asteroid.h ../src/main.h
object.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
object.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
object.o: ../src/headers.h ../src/glad/include/glad/glad.h
object.o: ../src/headers.h ../src/glad/include/glad/glad.h
object.o: ../src/object.h ../src/segment.h ../src/gpuProgram.h
object.o: ../src/segment.h ../src/gpuProgram.h ../src/seq.h
object.o: ../src/seq.h ../src/world.h ../src/ship.h ../src/shell.h
shell.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
shell.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
shell.o: ../src/glad/include/glad/glad.h
shell.o: ../src/headers.h ../src/glad/include/glad/glad.h
shell.o: ../src/object.h ../src/segment.h ../src/gpuProgram.h
shell.o: ../src/object.h ../src/segment.h ../src/gpuProgram.h
shell.o: ../src/seq.h
shell.o: ../src/seq.h
shell.o: ../src/shell.h ../src/headers.h
ship.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
ship.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
ship.o: ../src/headers.h ../src/glad/include/glad/glad.h
ship.o: ../src/object.h ../src/segment.h ../src/gpuProgram.h
ship.o: ../src/object.h ../src/segment.h ../src/gpuProgram.h
ship.o: ../src/seq.h ../src/shell.h
ship.o: ../src/seq.h ../src/shell.h
ship.o: ../src/ship.h ../src/headers.h ../src/glad/include/glad/glad.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/glad/include/glad/glad.h
strokefont.o: ../src/gpuProgram.h ../src/seq.h
strokefont.o: ../src/gpuProgram.h ../src/seq.h ../src/fg_stroke.h
strokefont.o: ../src/headers.h ../src/glad/include/glad/glad.h
strokefont.o: ../src/strokefont.h ../src/headers.h
world.o: ../src/asteroid.h ../src/main.h
world.o: ../src/asteroid.h ../src/main.h ../src/strokefont.h
world.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
world.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
world.o: ../src/glad/include/glad/glad.h
world.o: ../src/gpuProgram.h ../src/seq.h ../src/shell.h
world.o: ../src/gpuProgram.h ../src/seq.h ../src/shell.h
world.o: ../src/headers.h ../src/glad/include/glad/glad.h
world.o: ../src/ship.h ../src/object.h ../src/segment.h
world.o: ../src/ship.h ../src/object.h ../src/segment.h
world.o: ../src/world.h ../src/headers.h
