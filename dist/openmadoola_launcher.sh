#! /bin/bash

# OpenMadoola launcher
# Prompts the user for valid Wing of Madoola game data
# Requires zenity to be installed

# if the platform you're distributing on doesn't use xdg dirs, change this to "${HOME}"/.openmadoola
DATA_DIR="${XDG_DATA_HOME}"/openmadoola

function check_nes () {
    local good_checksum="dc541137d79000fe3e0bdf5ce35b687a5b137c3a09160c5742599b4ff257cfae"
    # skip first 16 bytes because iNES header can differ between different rom dumps
    local checksum=$( dd if="$1" bs=16 skip=1 | sha256sum | awk '{ print $1 }')
    if [ $checksum == $good_checksum ]; then
        echo "ROM validation succeeded"
        return 0
    else
        echo "ROM validation failed"
        return -1
    fi
}

function check_steam () {
    local good_checksum="6a7cd691eabecab58f5e9759d96ba01aab409ba4fd31a6b6e4e6aecb080164d5"
    echo "$good_checksum $1" | sha256sum --check --status
    if [ $? -eq 0 ]; then
        echo "Steam validation succeeded"
        return 0
    else
        echo "Steam validation failed"
        return -1
    fi
}

# make the data dir if it doesn't exist
if [ ! -d "$DATA_DIR" ]; then
    mkdir -p $DATA_DIR
fi

if [ -f "$DATA_DIR/madoola.nes" ]; then
    if ! check_nes "$DATA_DIR/madoola.nes"; then
        zenity --question --text="You have a Wing of Madoola ROM image installed that will not work with OpenMadoola. Delete it and select another ROM image?"
        if [ $? -eq 1 ]; then
            echo "User doesn't want to delete the invalid ROM image, quitting..."
            exit -1
        fi
        rm "$DATA_DIR/madoola.nes"
    fi
fi

if [ -f "$DATA_DIR/sharedassets0.assets" ]; then
    if ! check_steam "$DATA_DIR/sharedassets0.assets"; then
        zenity --question --text="You have a sharedassets0.assets file installed that will not work with OpenMadoola. Delete it and select another ROM image?"
        if [ $? -eq 1 ]; then
            echo "User doesn't want to delete the invalid ROM image, quitting..."
            exit -1
        fi
        rm "$DATA_DIR/sharedassets0.assets"
    fi
fi

if [ ! -f "$DATA_DIR/madoola.nes" ] && [ ! -f "$DATA_DIR/sharedassets0.assets" ]; then
    zenity --question --text="OpenMadoola requires a Wing of Madoola ROM image to function.\n\nThe ROM image can either be a standard NES ROM file or the sharedassets0.assets file from Sunsoft is Back! Retro Game Selection on Steam.\n\nWould you like to select your ROM image now?"
    if [ $? -eq 1 ]; then
        echo "No file selected, quitting..."
        exit -1
    fi

    valid_file=0
    while [ $valid_file -eq 0 ]; do
        file_path=$(zenity --file-selection --title="Select ROM image")
        if [ $? -eq 1 ]; then
            echo "File selection cancelled, quitting..."
            exit -1
        fi
        echo "User selected $file_path"

        if [[ ${file_path} == *sharedassets0.assets ]]; then
            if check_steam "${file_path}"; then
                cp "${file_path}" "$DATA_DIR/sharedassets0.assets"
                valid_file=1
            else
                zenity --question --text="Steam data not valid. It's possible that Sunsoft has released an update to the Steam collection that breaks compatibility with OpenMadoola. Please file a bug report if this is the case.\n\nTry another ROM image?"
                if [ $? -eq 1 ]; then
                    echo "File selection cancelled, quitting..."
                    exit -1
                fi
            fi
        else
            if check_nes "${file_path}"; then
                cp "${file_path}" "$DATA_DIR/madoola.nes"
                valid_file=1
            else
                zenity --question --text="Invalid ROM image selected. Try another ROM image?"
                if [ $? -eq 1 ]; then
                    echo "File selection cancelled, quitting..."
                    exit -1
                fi
            fi
        fi
    done
fi

openmadoola "$@"
