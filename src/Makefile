
CPP_SRC_FILES := main.cpp
CPP_FLAGS := -g -Wall -std=c++17
CPP_TARGET_NAME := multithreading


main: ${CPP_SRC_FILES}
	@echo Compiling ${CPP_SRC_FILES}
	g++ ${CPP_FLAGS} -o ${CPP_TARGET_NAME} ${CPP_SRC_FILES}

clean:
	@echo Removing files: ${CPP_TARGET_NAME}
	@rm ${CPP_TARGET_NAME}
