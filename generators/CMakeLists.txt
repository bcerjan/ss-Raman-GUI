# Ensures that we have a current header file for our PN code sequences
ADD_EXECUTABLE(pn_code_generator pn_code_generator.c)

ADD_CUSTOM_COMMAND(
  OUTPUT  pn_code.h
  COMMAND pn_code_generator
  DEPENDS pn_code_generator
)

ADD_CUSTOM_TARGET(pn_code DEPENDS pn_code.h)
