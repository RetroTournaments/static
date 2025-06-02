if [ -z "$1" ]; then
    echo "usage: $0 blah.mgl"
    exit 1
fi

echo "load_core $1" > /dev/MiSTer_cmd
