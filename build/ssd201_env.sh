#!/bin/sh

if [ "$1" == "reset" ]
then
    echo "Rest all PATH!!!"
    if [ -n "$BACK_SSD201_PATH" ]
      then
        export PATH="$BACK_SSD201_PATH"
        unset BACK_SSD201_PATH
    fi

    # unset CROSS_PATH
    unset RUN_SSD201_PATH
    echo PATH="$PATH"
    return
fi

CURRENT_DIR=$PWD

if [ "$RUN_SSD201_PATH" == "$CURRENT_DIR" ]
then
    echo "The PATH has been set, nothing done!!!"
else

  CROSS_PATH="/opt/ssd201/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf"
  RTT_EXEC_PATH=$CROSS_PATH"/bin"

  if [ -z "$BACK_SSD201_PATH" ]
  then
      BACK_SSD201_PATH=$PATH 
      export BACK_SSD201_PATH="$BACK_SSD201_PATH"
  fi

  PATH=$PATH:$RTT_EXEC_PATH 

  export PATH="$PATH"
  # export RTT_EXEC_PATH="$RTT_EXEC_PATH"
  # export CROSS_PATH="$CROSS_PATH"
  export RUN_SSD201_PATH="$CURRENT_DIR"

fi

echo BACK_SSD201_PATH=$BACK_SSD201_PATH
# echo CROSS_PATH=$CROSS_PATH
# echo RTT_EXEC_PATH=$RTT_EXEC_PATH
echo PATH=$PATH


