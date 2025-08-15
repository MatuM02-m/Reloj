MODULES = module/freertos
BOARD = edu-ciaa-nxp
MUJU = ./muju
DOC_DIR = ./build/doc

CFLAGS += -MMD -MP

include $(MUJU)/module/base/makefile

-include $(OBJECTS:.o=.d)

doc:
	@echo "Generando documentación con Doxygen..."
	mkdir -p $(DOC_DIR)
	doxygen Doxyfile
	