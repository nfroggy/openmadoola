## How to play MML files
OpenMadoola will look for MML files in its data directory (wherever you have the ROM image, font.bin, and nes.pal files).
If the files are named correctly (search for "soundFilenames" [here](../src/sound.cpp)), they'll override the regular music or sound effects.

You can also play MML files outside of the game by launching OpenMadoola with the `-p file.mml` option.

## Example files
If you composed anything and want it added here, feel free to submit a pull request.

- [doraemon.mml](doraemon.mml) - Doraemon: Giga Zombie no Gyakushuu (FC) - Underground Maze (cover by marklincadet)
- [precure.mml](precure.mml) - Futari Wa Pretty Cure (Sega Pico) - Title (cover by marklincadet)
- [sunset.mml](sunset.mml) - marklincadet - Running from the Sunset
- [spy.mml](spy.mml) - Spy vs. Spy (NES) - Main Theme (cover by marklincadet)

## OpenMadoola MML format
| Command           | Description                                                                                                                                                                             | Example                                               |
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------|
| `A[num]`          | Selects APU number. APU 0 is used for sound effects, and APU 1 is used for music. Can only be set once per file.                                                                        | `A1 ; music APU`                                      |
| `I[num]`          | Selects instrument number. There are 6 instruments, I0 to I5. Higher instrument numbers have a higher priority.                                                                         | `I0 ; instrument 0`                                   |
| `C[num]`          | Selects APU channel number. There are 4 channels, C0 to C3. C0 is pulse 1, C1 is pulse 2, C2 is triangle, and C3 is noise. An instrument's channel must be set before adding any notes. | `C0 ; Pulse 1`                                        |
| `R[num]:[hexval]` | Sets APU register 0 or 1 to the given value. APU register 0 is $4000 + (channel * 4), APU register 1 is $4001 + (channel * 4). Registers 0 and 1 must be set before adding any notes.   | `R0:cf ; set reg 0 to 0xcf`                           |
| `t[num]`          | Sets the tempo. Num is the number of frames (1 frame = 16.667ms) a 16th note can take. Tempo must be set before adding any notes.                                                       | `t5 ; 16th note takes 5 frames (180 bpm)`             |
| `l[num]`          | Sets the default note length. Must be set before adding any notes without lengths.                                                                                                      | `l8 ; default to 8th note`                            |
| `o[num]`          | Sets the octave for the instrument. An instrument's octave must be set before adding any notes. Valid octave numbers are between 1 and 8.                                               | `o4 ; octave 4`                                       |
| `L`               | Sets the loop point for the instrument. Instruments without loop points will only play once.                                                                                            | `L ; loop to here`                                    |
| `a[length][.]`    | Note command. The length parameter and dot are optional. Valid notes are c, c#, d, d#, e, f, f#, g, g#, a, a#, and b. Valid note lengths are 1, 2, 4, 8, and 16.                        | `c#4. ; dotted quarter note C#`                       |
| `r[length][.]`    | Rest command. The length parameter and dot are optional.                                                                                                                                | `r8 ; rest for the duration of an 8th note`           |                                                                                                                                                                  
| `^[length]`       | Tie command. Adds the length parameter to the previous note.                                                                                                                            | `f4^4^4 ; length of 3 quarter notes`                  |
| `>` and `<`       | `>` = octave up, `<` = octave down. Valid octave numbers are between 1 and 8.                                                                                                           | `g>g ; play g and g one octave higher`                |
| `N[hexval]`       | Adds a noise to the list. `val` gets written to $400e when the noise is played.                                                                                                         | `N6f ; add 0x6f to the noise list`                    |
| `n[num],[length]` | Plays a noise. `num` is the index of the noise in the list (i.e. the first noise added will be 0, the second will be 1, etc).                                                           | `n2,2 ; play noise 2 for the duration of a half note` |
| `[ ][num]`        | Loops `num` times. `num` must be between 2 and 16. Nested loops are not allowed.                                                                                                        | `[efg]5 ; plays e f g 5 times`                        |
| `;`               | Comment. Disregards everything after this point until the line break.                                                                                                                   | `; hello this is a comment`                           |



