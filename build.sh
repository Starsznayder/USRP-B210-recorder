SRC_PATH=$PWD
PROJ_PATH="/home/$USER/kitty"
PROJ_NAME="pk-b210-usrp3"
BUILD_TYPE="Release"


mkdir $PROJ_PATH
mkdir "$PROJ_PATH/rec"
rm -rf "$PROJ_PATH/bin"
rm -rf "$PROJ_PATH/include"
rm -rf "$PROJ_PATH/build"
mkdir "$PROJ_PATH/build"
mkdir "$PROJ_PATH/build/$PROJ_NAME"
mkdir "$PROJ_PATH/include"
mkdir "$PROJ_PATH/bin/"
cp -R "$SRC_PATH/utu" "$PROJ_PATH/include/utu"

cd "$PROJ_PATH/build/$PROJ_NAME"
cmake $SRC_PATH -DKITTY_INSTALL_PATH="$PROJ_PATH" -DCMAKE_BUILD_TYPE=Release
make -j6 && make install

cp -R "$SRC_PATH/src/config/ini" "$PROJ_PATH/bin/ini"

