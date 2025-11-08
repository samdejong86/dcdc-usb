CXX = g++
CXXFLAGS = -g -O2
CXXFLAGS += -Iinclude

MAXV=25
MINV=15

LINKER=g++

PACKAGES=libusb
LIBS=`pkg-config --libs ${PACKAGES}` -lm
INCS=`pkg-config --cflags ${PACKAGES}`

CXXFLAGS += $(LIBS)
CXXFLAGS += -lstdc++
CXXFLAGS += -DMAXV=${MAXV}
CXXFLAGS += -DMINV=${MINV}
CXXFLAGS += -Wall

LDFLAGS = -g -O2
LDFLAGS += -DMAXV=${MAXV}
LDFLAGS += -DMINV=${MINV}

SRC_DIR = src
OBJ_DIR = include

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

EXE = dcdc-usb


all: $(EXE)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c ${INCLUDE} $<



$(EXE): $(OBJ)
	${LINKER} -o $(EXE) ${LDFLAGS} ${OBJ} ${LIBS}

clean:
	rm ${EXE} ${OBJ}
