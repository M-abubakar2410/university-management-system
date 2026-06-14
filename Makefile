CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = academic_portal
SRC      = main.cpp

all: $(TARGET)

$(TARGET): $(SRC) MyString.h MyArray.h common.h assessment.h venue.h \
           entity.h course.h section.h database_manager.h scheduler.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	@echo ""
	@echo "  Build successful!  Run:  ./$(TARGET)"
	@echo ""

clean:
	rm -f $(TARGET)
