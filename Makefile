# lynxbot makefile

PROGNAME=lynxbot

INC=-I./include

LIB=-lcurl -lpthread

OBJ=obj
SRC=src
LIBD=lib

CXX=g++
CXXFLAGS=$(INC) -Wall -Wextra -c -std=c++14

# compile for debug or release (default: release)
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CXXFLAGS+=-DDEBUG -g
	OBJ=obj/debug
else
	CXXFLAGS+=-O2
	OBJ=obj/release
endif

_CPR=auth.o cookies.o cprtypes.o digest.o error.o multipart.o parameters.o\
    payload.o proxies.o session.o util.o
CPR=$(patsubst %,$(OBJ)/%,$(_CPR))
_CPRH=api.h auth.h body.h cookies.h cpr.h cprtypes.h curlholder.h defines.h\
     digest.h error.h multipart.h parameters.h payload.h proxies.h response.h\
     session.h timeout.h util.h
CPRH=$(patsubst %,include/cpr/%,$(_CPRH))
CPRD=$(LIBD)/cpr

JSONCPP=$(OBJ)/jsoncpp.o
_JSONH=json.h json-forwards.h
JSONH=$(patsubst %,include/json/%,$(_JSONH))
JSOND=$(LIBD)/json

_LYNXBOT=main.o utils.o client.o TwitchBot.o Moderator.o\
	URLParser.o CommandHandler.o CustomCommandHandler.o TimerManager.o\
	GEReader.o ExpressionParser.o OpMap.o OptionParser.o SelectionWheel.o\
	EventManager.o Giveaway.o SkillMap.o RSNList.o
LYNXBOT=$(patsubst %,$(OBJ)/%,$(_LYNXBOT))
_LBH=client.h CommandHandler.h CustomCommandHandler.h EventManager.h\
     ExpressionParser.h GEReader.h Giveaway.h Moderator.h OpMap.h\
     OptionParser.h RSNList.h SelectionWheel.h SkillMap.h TimerManager.h\
     TwitchBot.h URLParser.h utils.h version.h
LBH=$(patsubst %,$(SRC)/%,$(_LBH))

# lynxbot source
$(OBJ)/%.o: $(SRC)/%.cpp $(LBH)
	$(CXX) $(CXXFLAGS) -o $@ $<

# jsoncpp source
$(OBJ)/%.o: $(JSOND)/%.cpp $(JSONH)
	$(CXX) $(CXXFLAGS) -o $@ $<

# cpr source
$(OBJ)/%.o: $(CPRD)/%.cpp $(CPRH)
	$(CXX) $(CXXFLAGS) -o $@ $<

lynxbot: odir exec

# create directory for .o files
odir:
	@mkdir -p $(OBJ)

exec: $(CPR) $(JSONCPP) $(LYNXBOT)
	$(CXX) -o $(PROGNAME) $(LIB) $^

.PHONY: clean

clean:
	rm -f $(PROGNAME) *.o $(OBJ)/*.o