REM used to generate icon files. ImageMagick must be installed on your computer for this to work.

REM linux icon
magick lucia.png -resize 512x512 com.infochunk.OpenMadoola.png

REM windows icon
magick lucia.png -background transparent -resize 256x256 -density 256x256 openmadoola.ico

pause
