
require 'mkmf'

have_library('IL', 'ilInit') and
have_library('ILU', 'iluInit') and
  create_makefile('devil')
