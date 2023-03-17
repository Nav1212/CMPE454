LDFLAGS = -L. -lglfw -lGL -ldl # -lpthread
CXXFLAGS = -g -std=c++11 -Wall -Wno-write-strings -Wno-parentheses -Wno-unused-variable -Wno-unused-but-set-variable -Wno-maybe-uninitialized -DLINUX #-pthread

vpath %.cpp ../src
vpath %.c   ../src/glad/src

OBJS = main.o world.o sphere.o rectangle.o object.o gpuProgram.o linalg.o axes.o drawSegs.o strokefont.o fg_stroke.o glad.o

EXEC = anim

all:    $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS) 



clean:
	rm -f *~ $(EXEC) $(OBJS) Makefile.bak

depend:	
	makedepend -Y ../src/*.h ../src/*.cpp 2> /dev/null
	@echo ""
	@echo "Now edit the Makefile to remove the ../src/ prefix of all of the dependences for the .o files"
	@echo ""

# DO NOT DELETE

axes.o: ../src/linalg.h ../src/gpuProgram.h ../src/headers.h
axes.o: ../src/glad/include/glad/glad.h
axes.o: ../src/glad/include/KHR/khrplatform.h ../src/seq.h
drawSegs.o: ../src/headers.h ../src/glad/include/glad/glad.h
drawSegs.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
drawSegs.o: ../src/gpuProgram.h ../src/seq.h
fg_stroke.o: ../src/headers.h ../src/glad/include/glad/glad.h
fg_stroke.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/headers.h ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/seq.h
headers.o: ../src/glad/include/glad/glad.h
headers.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
main.o: ../src/linalg.h ../src/drawSegs.h ../src/headers.h
main.o: ../src/glad/include/glad/glad.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/gpuProgram.h
main.o: ../src/seq.h
object.o: ../src/headers.h ../src/glad/include/glad/glad.h
object.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
rectangle.o: ../src/linalg.h ../src/seq.h ../src/headers.h
rectangle.o: ../src/glad/include/glad/glad.h
rectangle.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
rectangle.o: ../src/gpuProgram.h
seq.o: ../src/headers.h ../src/glad/include/glad/glad.h
seq.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
sphere.o: ../src/linalg.h ../src/seq.h ../src/headers.h
sphere.o: ../src/glad/include/glad/glad.h
sphere.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
sphere.o: ../src/rectangle.h ../src/gpuProgram.h
strokefont.o: ../src/headers.h ../src/glad/include/glad/glad.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/gpuProgram.h ../src/seq.h
world.o: ../src/headers.h ../src/glad/include/glad/glad.h
world.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
world.o: ../src/sphere.h ../src/seq.h ../src/object.h
world.o: ../src/rectangle.h ../src/gpuProgram.h
axes.o: ../src/headers.h ../src/glad/include/glad/glad.h
axes.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
axes.o: ../src/axes.h ../src/gpuProgram.h ../src/seq.h
drawSegs.o: ../src/headers.h ../src/glad/include/glad/glad.h
drawSegs.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
drawSegs.o: ../src/drawSegs.h ../src/gpuProgram.h ../src/seq.h
fg_stroke.o: ../src/fg_stroke.h ../src/headers.h
fg_stroke.o: ../src/glad/include/glad/glad.h
fg_stroke.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/gpuProgram.h ../src/headers.h
gpuProgram.o: ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/seq.h
linalg.o: ../src/linalg.h
main.o: ../src/headers.h ../src/glad/include/glad/glad.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
main.o: ../src/seq.h ../src/axes.h ../src/gpuProgram.h
main.o: ../src/strokefont.h ../src/main.h ../src/drawSegs.h
main.o: ../src/world.h ../src/sphere.h ../src/object.h
main.o: ../src/rectangle.h
object.o: ../src/object.h ../src/headers.h
object.o: ../src/glad/include/glad/glad.h
object.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
rectangle.o: ../src/rectangle.h ../src/linalg.h ../src/seq.h
rectangle.o: ../src/headers.h ../src/glad/include/glad/glad.h
rectangle.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
rectangle.o: ../src/gpuProgram.h
sphere.o: ../src/sphere.h ../src/linalg.h ../src/seq.h
sphere.o: ../src/headers.h ../src/glad/include/glad/glad.h
sphere.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
sphere.o: ../src/rectangle.h ../src/gpuProgram.h
strokefont.o: ../src/strokefont.h ../src/headers.h
strokefont.o: ../src/glad/include/glad/glad.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/gpuProgram.h ../src/seq.h ../src/fg_stroke.h
world.o: ../src/world.h ../src/headers.h
world.o: ../src/glad/include/glad/glad.h
world.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
world.o: ../src/sphere.h ../src/seq.h ../src/object.h
world.o: ../src/rectangle.h ../src/gpuProgram.h ../src/main.h
world.o: ../src/drawSegs.h
