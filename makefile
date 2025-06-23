MODULES = 
BOARD = edu-ciaa-nxp
MUJU = ./muju
DOC_DIR = ./build/doc

include $(MUJU)/module/base/makefile

doc:
	@echo "Generando documentación con Doxygen..."
	mkdir -p $(DOC_DIR)
	doxygen Doxyfile
	