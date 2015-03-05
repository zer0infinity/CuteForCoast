"""
This SConscript defines several builders for converting file formats
needed for the the configured LaTeX build target.
"""
import os
import build_config

# Get the passed in environment.
Import('env')

## eps2pdf builder.
pdf_builder = Builder(action='epstopdf $SOURCE --outfile=$TARGET',
                      suffix=build_config.FILE_EXTENSIONS['pdf'],
                      src_suffix=build_config.FILE_EXTENSIONS['eps'])
env.Append(BUILDERS={'Eps2pdf': pdf_builder})

## pdf2eps builder.
eps_builder = Builder(action='pdftops -eps -level3 $SOURCE $TARGET',
                      suffix=build_config.FILE_EXTENSIONS['eps'],
                      src_suffix=build_config.FILE_EXTENSIONS['pdf'])
env.Append(BUILDERS={'Pdf2eps': eps_builder})

## png2eps builder. 
png_builder = Builder(action='convert $SOURCE $TARGET',
                      suffix=build_config.FILE_EXTENSIONS['eps'],
                      src_suffix=build_config.FILE_EXTENSIONS['png'])
env.Append(BUILDERS={'Png2eps': png_builder})

## jpg2eps builder. 
jpg_builder = Builder(action='convert $SOURCE $TARGET',
                      suffix=build_config.FILE_EXTENSIONS['eps'],
                      src_suffix=build_config.FILE_EXTENSIONS['jpg'])
env.Append(BUILDERS={'Jpg2eps': jpg_builder})

## GNUplot builder.
gnuplot_builder = Builder(action='gnuplot $SOURCE',
                          suffix=build_config.FILE_EXTENSIONS['eps'],
                          src_suffix=build_config.FILE_EXTENSIONS['gnuplot'])
env.Append(BUILDERS={'Gnuplot': gnuplot_builder})

## DOT builder.
dot_builder = Builder(action='dot -Tpng -o $TARGET $SOURCE',
                          suffix=build_config.FILE_EXTENSIONS['png'],
                          src_suffix=build_config.FILE_EXTENSIONS['dot'])
env.Append(BUILDERS={'Dot': dot_builder})

## MscGen builder.
mscgen_builder = Builder(action='mscgen -Tpng -o $TARGET $SOURCE',
                          suffix=build_config.FILE_EXTENSIONS['png'],
                          src_suffix=build_config.FILE_EXTENSIONS['msc'])
env.Append(BUILDERS={'Msc': mscgen_builder})

# Pass back the modified environment.
Return('env')
